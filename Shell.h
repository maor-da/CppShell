#pragma once
#include <CLI/CLI.hpp>
#include <concepts>
#include <string>

class Program
{
public:
	Program(std::string_view name, std::string_view desc = "");

	void add_program(std::shared_ptr<Program> prog);
	virtual void init() = 0;

	CLI::App_p app;
	std::set<std::string> completions;

protected:
	std::vector<std::shared_ptr<Program>> programs;
	virtual void exec() = 0;
};

class MainProgram : public Program
{
public:
	using base_t = Program;
	using base_t::base_t;
};

class Shell
{
public:
	// Shell(CLI::App_p app);
	//  Shell(std::string_view name, std::string_view desc);
	Shell(MainProgram& prog);
	~Shell();

	void run();

private:
	CLI::App_p app;
	std::string promptLine;
	bool exit = false;
	static bool setupTerminal;
	Program& program;

	Shell(Program& prog);
	void add_shell_commands();
	void remove_shell_commands();
	void update_autocomplete();

	friend Program;
};