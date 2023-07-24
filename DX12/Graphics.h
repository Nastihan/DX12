#pragma once
#include <cstdint>

class Graphics
{
public:
	Graphics(uint16_t width, uint16_t height);
private:
	uint16_t width;
	uint16_t height;
};