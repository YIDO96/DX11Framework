#pragma once

#include "Actor.h"

namespace Engine
{
	// 2-2 검증용 : 좌우로 왕복 이동하는 사각형 액터
	// 2-1에서 EngineApp에 있던 _quadX / _quadVelX 로직을 여기로 이동
	class TestMovingActor : public AActor
	{
	public:
		TestMovingActor() { _name = "TestMovingActor"; }

		void Tick(float deltaTime) override;

	private:
		float _velocityX = 0.4f;
	};
}