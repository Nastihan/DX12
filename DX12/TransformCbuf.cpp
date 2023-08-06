#include "TransformCbuf.h"
#include "Drawable.h"

TransformCbuf::TransformCbuf(const Drawable& parent)
	: pParent(&parent)
{
}

TransformCbuf::Transforms TransformCbuf::GetTransforms(Graphics& gfx)
{
	assert(pParent != nullptr);
	const auto model = pParent->GetTransform();
	const auto modelView = model * gfx.GetCamera();
	return {
		DirectX::XMMatrixTranspose(model),
		DirectX::XMMatrixTranspose(modelView),
		DirectX::XMMatrixTranspose(
			modelView *
			gfx.GetProjection()
		)
	};
}
