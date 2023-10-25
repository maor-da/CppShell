
#include <Windows.h>

#include <memory>
#include <string>

#include "RAII.h"

std::string GetHostname()
{
	int err;
	WSADATA wsaData;
	RAII initWSA(
		[&]() {
			int err = WSAStartup(MAKEWORD(2, 0), &wsaData);
			if (err != ERROR_SUCCESS) {
				// log
				return false;
			}
			return true;
		},
		[]() { WSACleanup(); });

	auto hn = std::make_unique<char[]>(256);

	int ret = gethostname(hn.get(), 256);
	if (ret != ERROR_SUCCESS) {
		// log error
	}

	return hn.get();
}