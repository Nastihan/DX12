#pragma once
#include <chrono>

class NastihanTimer
{
public:
	NastihanTimer();
	float Mark();
	float Peek();

private:
	std::chrono::steady_clock::time_point last;
};