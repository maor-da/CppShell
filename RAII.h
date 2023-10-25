#pragma once
#include <functional>

class RAII
{
public:
	RAII(std::function<bool()> init, std::function<void()> release) : m_release(release)
	{
		m_success = init();
	}

	~RAII()
	{
		if (m_success) {
			m_release();
		}
	}

private:
	bool m_success = false;
	std::function<void()> m_release;
};
