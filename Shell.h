#pragma once
#include <CLI/CLI.hpp>
#include <concepts>
#include <string>

class Program
{
public:
	Program(std::string_view name, std::string_view desc = "");

	inline void attach(CLI::App_p parent)
	{
		parent->add_subcommand(app);
	}
	virtual CLI::App_p init() = 0;

protected:
	virtual void exec() = 0;

	CLI::App_p app;
};

class Shell
{
public:
	Shell(CLI::App_p app);
	Shell(std::string_view name, std::string_view desc);
	Shell(std::shared_ptr<Program> prog);

	void add_program(std::shared_ptr<Program> prog)
	{
		app->add_subcommand(prog->init());
		programs.push_back(prog);
	}

	void run();

private:
	CLI::App_p app;
	std::string promptLine;
	bool exit = false;
	static bool setupTerminal;
	std::vector<std::shared_ptr<Program>> programs;
};