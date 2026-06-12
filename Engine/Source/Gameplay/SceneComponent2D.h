#pragma once

#include "Gameplay/Object.h"
#include <DirectXMath.h>

namespace Engine
{
	// 위치/회전/크기(Transform)를 소유하는 컴포넌트
	// 액터는 이 컴포넌트를 통해서만 월드 공간에 존재
	class USceneComponent2D : public UObject
	{
	public:
		USceneComponent2D() { _name = "SceneComponent2D"; }

		// --- 위치 ---
		void SetPosition(float x, float y) { _position = { x, y }; }
		void SetPosition(const DirectX::XMFLOAT2& p) { _position = p; }
		const DirectX::XMFLOAT2& GetPosition() const { return _position; }
		
		void AddPosition(float dx, float dy) { _position.x += dx; _position.y += dy; }

		// --- 회전(라디안) / 크기 : 자리만 마련, 이후 단계에서 행렬에 반영
		void SetRotation(float radians) { _rotation = radians; }
		float GetRotation() const { return _rotation; }

		void SetScale(float sx, float sy) { _scale = { sx, sy }; }
		const DirectX::XMFLOAT2& GetScale() const{ return _scale; }


	private:
		DirectX::XMFLOAT2	_position = { 0.0f, 0.0f };
		float				_rotation = 0.0f;
		DirectX::XMFLOAT2	_scale = { 1.0f, 1.0f };
	};
}