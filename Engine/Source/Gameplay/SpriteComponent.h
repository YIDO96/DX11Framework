#pragma once

#include "SceneComponent2D.h"

namespace Engine
{
	class Texture;
	class GraphicsRHI;


	class USpriteComponent : public USceneComponent2D
	{
	public:
		USpriteComponent() { _name = "SpriteComponent"; }

		void SetTexture(const Texture* texture) { _texture = texture; }
		const Texture* GetTexture() const { return _texture; }

		void SetSize(float w, float h) { _size = { w, h }; }

		// 지정된 월드 위치에 자신의 텍스터를 렌더러로 제출
		void Draw(GraphicsRHI* rhi, float worldX, float worldY) const;

	private:
		const Texture* _texture = nullptr;				// 참조만 (소유 x)
		DirectX::XMFLOAT2 _size = { 0.4f, 0.4f };		// 화면상 크기 (NDC)
	};
}