#include "pch.h"
#include "Texture.h"

#include <DirectXTex.h>

using namespace DirectX;

namespace Engine
{
	bool Texture::LoadFromFile(ID3D11Device* device, const std::wstring& path)
	{
		if (!device)
			return false;


		ScratchImage image;
		HRESULT hr = LoadFromWICFile(path.c_str(), WIC_FLAGS_NONE, nullptr, image);
		if (FAILED(hr))
		{
			OutputDebugStringA("Texture::LoadFromFile FAILED (check path)\n");
			return false;
		}

		// 크기 정보 보관 (나중에 스프라이트 크기 자동 계상 등에 사용)
		const TexMetadata& meta = image.GetMetadata();
		_width = static_cast<int>(meta.width);
		_height = static_cast<int>(meta.height);


		hr = CreateShaderResourceView(device, image.GetImages(), image.GetImageCount(), meta, _srv.GetAddressOf());

		return SUCCEEDED(hr);
	}
}