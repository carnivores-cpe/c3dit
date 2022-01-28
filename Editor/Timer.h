#pragma once

#include <chrono>


class Timer
{
private:
	std::chrono::steady_clock::duration m_TimeStart;

public:

	Timer()
	{
		m_TimeStart = std::chrono::steady_clock::now().time_since_epoch();
	}

	/* Replacement for winmm.lib : timeGetTime() */
	int64_t GetTime()
	{
		auto t = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch() - m_TimeStart);
		return t.count();
	}

};