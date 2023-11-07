#pragma once
#include <CLI/CLI.hpp>
#include <concepts>
#include <string>

class Program
{
public:
	Program(std::string_view name, std::string_view desc = "");

	void add_program(std::shared_ptr<Program> prog);
	virtual CLI::App_p init() = 0;

protected:
	std::vector<std::shared_ptr<Program>> programs;
	CLI::App_p app;

	virtual void exec() = 0;
};

class Shell
{
public:
	Shell(CLI::App_p app);
	// Shell(std::string_view name, std::string_view desc);
	Shell(std::shared_ptr<Program> prog);

	void add_shell_commands();
	void update_autocomplete();

	void add_program(std::shared_ptr<Program> prog);

	void run();

private:
	CLI::App_p app;
	std::string promptLine;
	bool exit = false;
	static bool setupTerminal;
	std::set<std::string> completions;
	std::vector<std::shared_ptr<Program>> programs;
};