#include "Shell.h"

#ifndef CPPSHELL_LIB
#ifdef _DEBUG

class Sub : public Program
{
public:
	using base_t = Program;

	Sub(std::string_view name, std::string_view desc) : base_t(name, desc) {}

	virtual void init() override
	{
		app->add_option("--val1", val1, "val1 desc")->ignore_case();
	}

private:
	std::string val1;

	virtual void exec() override
	{
		printf("exec %s\n", val1.c_str());
	}
};

class Sub2 : public Program
{
public:
	using base_t = Program;

	Sub2() : base_t("sub2") {}

	virtual void init() override
	{
		app->add_option("--val1", val1, "val1 desc")->ignore_case()->required();
	}

private:
	std::string val1;

	virtual void exec() override
	{
		printf("exec %s\n", val1.c_str());
	}
};

class Main : public MainProgram
{
public:
	using base_t = MainProgram;

	Main() : base_t("main") {}

	virtual void init() override
	{
		auto sub1 = std::make_shared<Sub>("add", "adding value");
		auto sub2 = std::make_shared<Sub2>();

		add_program(sub1);
		add_program(sub2);

		app->add_option("--val1", val1, "val1 desc")->ignore_case()->required();
	}

private:
	std::string val1;

	virtual void exec() override
	{
		printf("exec %s\n", val1.c_str());
	}
};

int main(int argc, char** argv)
{
	// Shell app("Base", "The top most app");
	Main main;
	Shell app(main);

	app.run();

	return 0;
}

#endif	// DEBUG

#endif	// !CPPSHELL_LIB
