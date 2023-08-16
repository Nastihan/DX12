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

TransformCbuf::TransformsRT TransformCbuf::GetTransformsRT(Graphics& gfx)
{
	assert(pParent != nullptr);
	const auto view = gfx.GetCamera();
	DirectX::XMVECTOR determinant;
	return {
		DirectX::XMMatrixTranspose(view),
		DirectX::XMMatrixTranspose(
			view *
			gfx.GetProjection()
		),
		DirectX::XMMatrixTranspose(
			DirectX::XMMatrixInverse(
				&determinant, view
			)),
		DirectX::XMMatrixTranspose(
			DirectX::XMMatrixInverse(
				&determinant,( view *
					gfx.GetProjection())
			))
	};
}
