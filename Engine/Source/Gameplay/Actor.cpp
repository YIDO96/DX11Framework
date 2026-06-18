#include "pch.h"
#include "Actor.h"
#include "Grid/GridSystem.h"


namespace Engine
{
	AActor::AActor()
	{
		_name = "Actor";

		// 액터 생성과 동시에 RootComponent를 가진다 -> 비로소 월드 공간에 존재
		_rootComponent	= std::make_unique<USceneComponent2D>();
		_sprite			= std::make_unique<USpriteComponent>();
	}
	void AActor::Render(GraphicsRHI* rhi, const GridSystem* grid)
	{
		if (!_sprite)
			return;

		// 격좌 좌표 -> 화면(NDC) 변환
		const DirectX::XMFLOAT2& gridPos = _rootComponent->GetPosition();
		DirectX::XMFLOAT2 ndc = grid->GridToScreen(gridPos.x, gridPos.y);

		// 타일 한 칸 크기에 맞춰 스프라이트 스케일 결정
		DirectX::XMFLOAT2 tileNDC = grid->TileSizeInNDC();
		_sprite->SetSize(tileNDC.x, tileNDC.y);


		// 루트 컴포넌트의 위치를 스프라이트에 전달하여 제출
		//const DirectX::XMFLOAT2& pos = _rootComponent->GetPosition();
		_sprite->Draw(rhi, ndc.x, ndc.y);
	}
}