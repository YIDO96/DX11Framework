#include "pch.h"
#include "SpriteComponent.h"
#include "RHI/GraphicsRHI.h"

namespace Engine
{
	void USpriteComponent::Draw(GraphicsRHI* rhi, float worldX, float worldY) const
	{
		if (!rhi || !_texture)
			return;

		rhi->DrawSprite(_texture, worldX, worldY, _size.x, _size.y);
	}
}

