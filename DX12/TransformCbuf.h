#pragma once
#include <DirectXMath.h>
#include "Graphics.h"

class Drawable;

class TransformCbuf
{
private:
	struct Transforms
	{
		DirectX::XMMATRIX model;
		DirectX::XMMATRIX modelView;
		DirectX::XMMATRIX modelViewProj;
	};
public:
	TransformCbuf(const Drawable& parent);
	Transforms GetTransforms(Graphics& gfx);
private:
	const Drawable* pParent = nullptr;
};
