#pragma once

#include "Gameplay/Object.h"
#include "Gameplay/SceneComponent2D.h"
#include <DirectXMath.h>
#include <memory>

namespace Engine
{
	class UInputSubsystem;

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

		// --- 편의 : 위치 접근을 RootComponent로 위임 ---
		void SetPosition(float x, float y) { _rootComponent->SetPosition(x, y); }
		const DirectX::XMFLOAT2 GetPosition() const { return _rootComponent->GetPosition(); }

		USceneComponent2D* GetRootComponent() const { return _rootComponent.get(); }

		void SetInput(const UInputSubsystem* input) { _input = input; }
	protected:
		std::unique_ptr<USceneComponent2D> _rootComponent;
		const UInputSubsystem* _input = nullptr;
	};
}