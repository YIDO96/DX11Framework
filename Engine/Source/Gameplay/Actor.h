#pragma once

#include "Gameplay/Object.h"
#include "Gameplay/SceneComponent2D.h"
#include "Gameplay/SpriteComponent.h"
#include <DirectXMath.h>
#include <memory>

namespace Engine
{
	class UInputSubsystem;
	class GraphicsRHI;

	// 월드에 소폰 가능한 액체. RootComponent(SceneComponent2D)를 소유
	class AActor : public UObject
	{
	public:
		AActor();
		virtual ~AActor() = default;

		// 스폰 직후 1회 호출
		virtual void BeginPlay() { }

		// 매 프레임 호출 자식이 override해서 자기 로직 작성
		virtual void Tick(float deltaTime) { }

		// 자신의 스프라이트를 렌더러에 제출
		virtual void Render(GraphicsRHI* rhi);



		// --- 편의 : 위치 접근을 RootComponent로 위임 ---
		void SetPosition(float x, float y) { _rootComponent->SetPosition(x, y); }
		const DirectX::XMFLOAT2 GetPosition()	const { return _rootComponent->GetPosition(); }

		USceneComponent2D* GetRootComponent()	const { return _rootComponent.get(); }
		USpriteComponent* GetSprite()			const { return _sprite.get(); }

		void SetInput(const UInputSubsystem* input) { _input = input; }


	protected:
		std::unique_ptr<USceneComponent2D> _rootComponent;
		std::unique_ptr<USpriteComponent> _sprite;


		const UInputSubsystem* _input = nullptr;
	};
}