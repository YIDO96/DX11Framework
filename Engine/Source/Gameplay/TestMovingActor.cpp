#include "pch.h"
#include "TestMovingActor.h"

namespace Engine
{
	void TestMovingActor::Tick(float deltaTime)
	{
		// 현재 위치를 가져와 프레임 독립적으로 이동
		DirectX::XMFLOAT2 pos = GetPosition();
		pos.x += _velocityX * deltaTime;

		// 양끝에서 방향 전환 (왕복)
		if (pos.x > 0.7f) { pos.x = 0.7f;  _velocityX = -_velocityX; }
		if (pos.x < -0.7f) { pos.x = -0.7f; _velocityX = -_velocityX; }

		SetPosition(pos.x, pos.y);
	}
}