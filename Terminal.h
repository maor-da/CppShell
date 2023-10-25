#pragma once
#include <string>

std::string CreatePromptLine(std::string_view appName, std::string_view cwd, std::string_view host);

bool SetupTerminal();
