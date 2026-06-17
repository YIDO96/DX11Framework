#include "pch.h"
#include "Actor.h"

namespace Engine
{
	AActor::AActor()
	{
		_name = "Actor";

		// 액터 생성과 동시에 RootComponent를 가진다 -> 비로소 월드 공간에 존재
		_rootComponent	= std::make_unique<USceneComponent2D>();
		_sprite			= std::make_unique<USpriteComponent>();
	}
	void AActor::Render(GraphicsRHI* rhi)
	{
		if (!_sprite)
			return;

		// 루트 컴포넌트의 위치를 스프라이트에 전달하여 제출
		const DirectX::XMFLOAT2& pos = _rootComponent->GetPosition();
		_sprite->Draw(rhi, pos.x, pos.y);
	}
}