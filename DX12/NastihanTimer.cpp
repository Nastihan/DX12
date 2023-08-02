#include "NastihanTimer.h"

NastihanTimer::NastihanTimer()
{
	last = std::chrono::steady_clock::now();
}

float NastihanTimer::Mark()
{
	const auto old = last;
	last = std::chrono::steady_clock::now();
	return std::chrono::duration<float>(std::chrono::steady_clock::now() - old).count();
}

float NastihanTimer::Peek()
{
	return std::chrono::duration<float>(std::chrono::steady_clock::now() - last).count();
}
