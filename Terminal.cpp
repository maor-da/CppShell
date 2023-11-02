
#include "Terminal.h"

#include <rang.hpp>
#include <sstream>

#pragma execution_character_set("utf-8")

std::string CreatePromptLine(std::string_view appName, std::string_view cwd, std::string_view host)
{
	std::stringstream ss;
	ss << rang::style::bold << rang::fg::green << "┌──(" << rang::fg::blue << appName << '@' << host
	   << rang::fg::green << ")-[" << rang::fg::reset << cwd << rang::fg::green << "]" << std::endl
	   << rang::fg::reset << rang::style::reset;

	return ss.str();
}

bool SetupTerminal()
{
	do {
		HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
		if (hOut == INVALID_HANDLE_VALUE) {
			break;
		}

		DWORD dwMode = 0;
		if (!GetConsoleMode(hOut, &dwMode)) {
			break;
		}

		dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING | DISABLE_NEWLINE_AUTO_RETURN;
		if (!SetConsoleMode(hOut, dwMode)) {
			dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
			if (!SetConsoleMode(hOut, dwMode)) {
				break;
			}
		}

		if (!SetConsoleOutputCP(CP_UTF8)) {
			break;
		}

		rang::setControlMode(rang::control::Force);
		rang::setWinTermMode(rang::winTerm::Ansi);

		return true;

	} while (false);

	return false;
}