#pragma once
#include <source_location>


struct CheckerToken{};
extern CheckerToken chk;
struct HrGrabber {
	HrGrabber(unsigned int hr, std::source_location = std::source_location::current()) noexcept;
	unsigned int hr;
	std::source_location loc;
};
void operator>>(HrGrabber, CheckerToken);


