#include "pch.h"
#include "TestMovingActor.h"
#include "Subsystems/InputSubsystem.h"


namespace Engine
{
	void TestMovingActor::Tick(float deltaTime)
	{
        if (!_input)
            return;   // 입력이 연결 안 됐으면 아무것도 안 함

        float dx = 0.0f;
        float dy = 0.0f;
        
        // 눌려 있는 동안 계속 이동 (IsKeyDown)
        if (_input->IsKeyDown(VK_LEFT))     dx -= 1.0f;
        if (_input->IsKeyDown(VK_RIGHT))    dx += 1.0f;
        if (_input->IsKeyDown(VK_UP))       dy += 1.0f;   // NDC는 위가 +Y
        if (_input->IsKeyDown(VK_DOWN))     dy -= 1.0f;

        //// 프레임 독립적 이동: 방향 × 속도 × dt
        //DirectX::XMFLOAT2 pos = GetPosition();
        //pos.x += dx * _speed * deltaTime;
        //pos.y += dy * _speed * deltaTime;
        //
        //// 화면 밖으로 못 나가게 NDC 범위로 제한
        //if (pos.x > 0.9f) pos.x = 0.9f;
        //if (pos.x < -0.9f) pos.x = -0.9f;
        //if (pos.y > 0.9f) pos.y = 0.9f;
        //if (pos.y < -0.9f) pos.y = -0.9f;

        DirectX::XMFLOAT2 pos = GetPosition();
        pos.x += dx * _speed * deltaTime;
        pos.y += dy * _speed * deltaTime;


        SetPosition(pos.x, pos.y);
	}
}