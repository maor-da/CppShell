#include "Shell.h"

#include <CLI/CLI.hpp>
#include <memory>
#include <rang.hpp>
#include <ranges>
#include <string>

#include "Host.h"
#include "Shell.h"
#include "Terminal.h"
#include "linenoise.hpp"

#pragma execution_character_set("utf-8")

bool Shell::setupTerminal = SetupTerminal();
std::string historyFile	  = (std::filesystem::current_path() / ".history").string();

std::set<std::string> builtins;
std::string promptStart = (std::stringstream() << rang::style::bold << rang::fg::green << "└─$ "
											   << rang::fg::reset << rang::style::reset)
							  .str();

void remove_subcommand(CLI::App* app)
{
	auto parent = app->get_parent();
	parent->remove_subcommand(app);
};

template <typename T>
void null_deleter(T* _)
{
}

class ShellFormatter : public CLI::Formatter
{
public:
	using base_t = CLI::Formatter;

	ShellFormatter()
	{
		label("REQUIRED", (std::stringstream{} << rang::fg::red << "[required]" << rang::fg::reset).str());
	}

	std::string make_positionals(const CLI::App* app) const override
	{
		return "";
	}
};

Program::Program(std::string_view name, std::string_view desc)
{
	if (name.empty()) {
		throw std::invalid_argument("Name is missing");
	}

	app = std::make_shared<CLI::App>(desc.data(), name.data());
	// app->positionals_at_end();

	auto guard = std::make_shared<bool>(false);
	app->preparse_callback([this, guard](size_t size) {
		if (size == 0 && *guard == false) {
			*guard = true;
			Shell{*this}.run();
			*guard = false;
			throw CLI::Success();
		}
	});

	app->callback([this]() { exec(); });

	app->formatter(std::make_shared<ShellFormatter>());

	app->add_subcommand("help", "Print this help message and exit")
		->ignore_case()
		->preparse_callback([app = app](size_t size) {
			std::cout << app->get_formatter()->make_help(
				app.get(), app->get_name(), CLI::AppFormatMode::Normal);
			throw CLI::Success();
		})
		->alias("?")
		->group("");
	completions.insert("help");
	completions.insert("?");

	completions.insert(builtins.begin(), builtins.end());
}

void Program::add_program(std::shared_ptr<Program> prog)
{
	if (prog == nullptr || prog->app == nullptr) {
		throw std::runtime_error("Bad program");
	}
	app->add_subcommand(prog->app);
	prog->init();
	programs.push_back(prog);

	completions.insert(prog->app->get_name());
}

// execute when added and everytime it runs
Shell::Shell(Program& prog) : program(prog), app(prog.app)
{
	if (app == nullptr) {
		throw std::runtime_error("App wasn't initialized");
	}

	if (!setupTerminal) {
		throw std::runtime_error("Terminal setup has failed");
	}

	app->parse_complete_callback([]() {
		// std::cout << std::endl;
		std::cin.clear();
	});

	add_shell_commands();
	update_autocomplete();
}

// Shell::Shell(std::string_view name, std::string_view desc)
//	: Shell(std::make_shared<CLI::App>(desc.data(), name.data()))
//{
// }

// executed only once, the base shell
Shell::Shell(MainProgram& prog) : Shell(static_cast<Program&>(prog))
{
	prog.init();

	program.completions.insert(builtins.begin(), builtins.end());

	linenoise::SetMultiLine(true);
	linenoise::SetHistoryMaxLen(40);
	linenoise::LoadHistory(historyFile.c_str());
}

Shell::~Shell()
{
	remove_shell_commands();
}

void Shell::add_shell_commands()
{
	try {
		app->remove_subcommand(app->get_subcommand("exit"));
	} catch (const std::exception&) {
	}

	app->add_subcommand("exit")
		->ignore_case()
		->preparse_callback([this](size_t size) {  // this require to update every time
			exit = true;
			throw CLI::Success();
		})
		->alias("quit")
		->group("");
	builtins.insert("exit");
	builtins.insert("quit");

	if (app->get_subcommands([](CLI::App* p) { return p->get_name() == "cd"; }).empty()) {
		app->add_subcommand("cd", "Change directory")
			->ignore_case()
			->group("")
			->preparse_callback([](size_t size) {
				if (size > 1) {
					throw CLI::ExtrasError({});
				}
			})
			->add_option_function<std::string>("path", [app = app](std::string path) {
				try {
					std::filesystem::current_path(path);
				} catch (const std::filesystem::filesystem_error& e) {
					std::cout << "The system cannot find the file specified: " << e.path1() << std::endl;
				}
			});
		builtins.insert("cd");
	}
}

void Shell::remove_shell_commands()
{
	try {
		app->remove_subcommand(app->get_subcommand("exit"));
	} catch (const std::exception&) {
	}

	try {
		app->remove_subcommand(app->get_subcommand("cd"));
	} catch (const std::exception&) {
	}
}



void Shell::update_autocomplete()
{
	linenoise::SetCompletionCallback([this](std::string_view editBuffer, std::vector<std::string>& options) {
		if (editBuffer.starts_with("cd ")) {
			std::string arg		 = ".";
			std::string_view esc = "/\\ \"'`";
			auto start			 = editBuffer.find_first_not_of(esc.substr(2), 2);
			auto end			 = editBuffer.length();

			if (start != std::string::npos) {
				for (const auto& c : editBuffer | std::views::reverse) {
					if ((end - start) == 0 || esc.find(c) == std::string::npos) {
						break;
					}
					end--;
				}
				arg = editBuffer.substr(start, end - start);
			}

			std::filesystem::path path = arg + "/";

			while (!path.empty() && !std::filesystem::exists(path)) {
				path /= "../";
				path = path.lexically_normal();
				arg	 = path.string();
			}

			if (path.is_relative()) {
				path = std::filesystem::current_path() / path;
			}

			if (path.root_directory() == arg) {
				arg = path.string();
			}

			try {
				for (const auto& entry : std::filesystem::directory_iterator(path)) {
					if (entry.is_directory()) {
						std::filesystem::path suggest = arg + "/" + entry.path().filename().string();
						options.push_back("cd '" + suggest.lexically_normal().string() + "'");
					}
				}
			} catch (const std::exception&) {
			}

			return;
		}

		//for ()

			// auto start = completions.lower_bound(editBuffer.data());
			for (auto it = program.completions.lower_bound(editBuffer.data());
				 it != program.completions.end();
				 it++) {
				if (it->starts_with(editBuffer)) {
					options.push_back(*it);
				} else {
					break;
				}
			}
	});
}

void Shell::run()
{
	while (!exit) {
		std::string line;

		promptLine = CreatePromptLine(
			app->get_name(), std::filesystem::current_path().string(), GetHostname().c_str());
		std::cout << std::endl;
		std::cout << rang::bg::reset << promptLine;

		update_autocomplete();
		linenoise::Readline(promptStart.c_str(), line);
		if (line.empty()) {
			continue;
		}
		linenoise::AddHistory(line.c_str());

		if (program.completions.contains(line)) {
			// deal with positional args
			line = "++ " + line;
		}

		try {
			app->parse(line);
		} catch (const CLI::CallForHelp& e) {
			std::cout << app->help();
		} catch (const CLI::ExtrasError& e) {
			app->exit(e);
		} catch (const CLI::ParseError& e) {
			if (e.get_exit_code() != 0) {
				app->exit(e);
				std::cout << app->help();
			}
		} catch (const CLI::Error& e) {
			app->exit(e);
		}

		linenoise::SaveHistory(historyFile.c_str());
	}
}
