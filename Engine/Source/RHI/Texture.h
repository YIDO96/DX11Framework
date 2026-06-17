#pragma once

#include <d3d11.h>
#include <wrl/client.h>
#include <string>

namespace Engine
{
	class Texture
	{
	public:
		Texture() = default;
		~Texture() = default;

		// 파일을 로딩해 SRV 생성, 성공 시 true
		bool LoadFromFile(ID3D11Device* device, const std::wstring& path);

		ID3D11ShaderResourceView* GetSRV() const { return _srv.Get(); }
		bool IsValid() const { return _srv != nullptr; }

		int GetWidth() const { return _width; }
		int GetHeight() const { return _height; }


	private:
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> _srv;
		int _width		= 0;
		int _height		= 0;
	};
}