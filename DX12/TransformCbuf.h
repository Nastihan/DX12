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
	struct TransformsRT
	{
		DirectX::XMMATRIX View;
		DirectX::XMMATRIX ViewProj;
		DirectX::XMMATRIX ViewI;
		DirectX::XMMATRIX ViewProjI;
	};

public:
	TransformCbuf(const Drawable& parent);
	Transforms GetTransforms(Graphics& gfx);
	TransformsRT GetTransformsRT(Graphics& gfx);

private:
	const Drawable* pParent = nullptr;
};
