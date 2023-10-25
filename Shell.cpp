#include "Shell.h"
#include "Shell.h"
#include "Shell.h"

#include <CLI/CLI.hpp>
#include <memory>
#include <rang.hpp>
#include <string>

#include "Host.h"
#include "Terminal.h"

bool Shell::setupTerminal = SetupTerminal();

void remove_subcommand(CLI::App* app)
{
	auto parent = app->get_parent();
	parent->remove_subcommand(app);
};

template <typename T>
void null_deleter(T* _)
{
}

void AddHelp(CLI::App_p app)
{
	if (app->get_subcommands([](CLI::App* p) { return p->get_name() == "help"; }).empty()) {
		auto help = app->add_subcommand("help", "Print this help message and exit")
						->ignore_case()
						->preparse_callback([app](size_t size) {
							std::cout << app->get_formatter()->make_help(
								app.get(), app->get_name(), CLI::AppFormatMode::Normal);
							throw CLI::Success();
						});
		help->alias("?");
	}
}

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
			Shell s(app);
			s.run();
			*guard = false;
			throw CLI::Success();
		}
	});

	AddHelp(app);
	app->callback([this]() { exec(); });
}

void Program::add_program(std::shared_ptr<Program> prog)
{
	if (prog == nullptr) {
		return;
	}
	app->add_subcommand(prog->init());
	programs.push_back(prog);
}

Shell::Shell(CLI::App_p app) : app(app)
{
	if (app == nullptr) {
		return;
	}

	if (!setupTerminal) {
		throw std::runtime_error("Terminal setup has failed");
	}

	promptLine =
		CreatePromptLine(app->get_name(), std::filesystem::current_path().string(), GetHostname().c_str());

	app->parse_complete_callback([]() {
		// std::cout << std::endl;
		std::cin.clear();
	});

	if (app->get_subcommands([](CLI::App* p) { return p->get_name() == "exit"; }).empty()) {
		app->add_subcommand("exit")->ignore_case()->preparse_callback([this](size_t size) {
			exit = true;
			throw CLI::Success();
		});
	}

	AddHelp(app);
}

Shell::Shell(std::string_view name, std::string_view desc)
	: Shell(std::make_shared<CLI::App>(desc.data(), name.data()))
{
}

Shell::Shell(std::shared_ptr<Program> prog) : Shell(prog->init())
{
	app->preparse_callback([](size_t size) {});
	programs.push_back(prog);
}


void Shell::run()
{
	std::string line;

	while (!exit) {
		std::cout << std::endl;
		std::cout << rang::bg::reset << promptLine;
		std::getline(std::cin, line);
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
	}
}
