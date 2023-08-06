#pragma once
#include "Graphics.h"

class Drawable
{
public:
	Drawable() = default;
	Drawable(const Drawable&) = delete;
	virtual DirectX::XMMATRIX GetTransform() const noexcept = 0;
	virtual void Draw(Graphics& gfx) const = 0;
	virtual ~Drawable() = default;
};