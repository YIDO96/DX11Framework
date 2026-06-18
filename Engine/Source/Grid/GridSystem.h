#pragma once

#include <DirectXMath.h>

namespace Engine
{
	// 격자 좌표계 + 좌표 변환 유틸
	//
	// 좌표 약속 :
	//		- 격자 원점 (0,0) = 맵 좌상단
	//		- gx -> 오른쪽 증가, gy -> 아래 증가
	//
	// 변환 단계 : 격자(Grid) -> 월드 픽셀 (World) -> NDC
	//		지금은 카메라/투영이 없으므로 World->NDC를 화면 크기로 직접 계산
	//		Phase 5에서 카메라 (투영 행렬)가 들어오면 클래스 내부 수정

	class GridSystem
	{
	public:
		GridSystem(int screenWidth, int screenHeight, int tilePixelSize = 32,
					int mapCols = 20, int mapRows = 20)
			: _screenW(screenWidth), _screenH(screenHeight), _tileSize(tilePixelSize),
			_cols(mapCols), _rows(mapRows)
		{ }

		// 격좌 좌표 (float) -> 월드 픽셀 좌표 (타일 중심 기준)
		// 맵을 화면 중앙에 배치하기 위해 맵 전체 픽셀 크기의 절반만큼 보정
		DirectX::XMFLOAT2 GridToWorld(float gx, float gy) const
		{
			const float mapW = _cols * (float)_tileSize;
			const float mapH = _rows * (float)_tileSize;

			// 격자 좌표를 픽셀로, 맵 중앙이 원점에 오도록 이동
			float worldX = gx * _tileSize - mapW * 0.5f + _tileSize * 0.5f;
			float worldY = gy * _tileSize - mapH * 0.5f + _tileSize * 0.5f;

			return { worldX, worldY };
		}

		// 월드 픽셀 좌표 -> NDC (-1 ~ 1). 임시 투영 (화면 크기로 정규화)
		// Y는 화면이 위가 +이고, 월드는 아래가 +이므로 부호는 반전
		DirectX::XMFLOAT2 WorldToNDC(float wx, float wy) const
		{
			float ndcX = (wx / (_screenW * 0.5f));
			float ndcY = (wy / (_screenH * 0.5f));

			return { ndcX , ndcY };
		}

		// 격좌 좌표 -> NDC (위 둘을 함친 것). 렌더 시 사용하는 함수
		DirectX::XMFLOAT2 GridToScreen(float gx, float gy) const
		{
			DirectX::XMFLOAT2 w = GridToWorld(gx, gy);
			return WorldToNDC(w.x, w.y);
		}

		// 타일 한 칸이 NDC 상에서 차지하는 크기 (스프라이트 스케일 계산용)
		DirectX::XMFLOAT2 TileSizeInNDC() const
		{
			float w = _tileSize / (_screenW * 0.5f);
			float h = _tileSize / (_screenH * 0.5f);

			return { w, h };
		}



		int GetCols() const { return _cols; }
		int GetRows() const { return _rows; }
		int GetTileSize() const { return _tileSize; }

	private:
		int _screenW, _screenH;
		int _tileSize;
		int _cols, _rows;
	};
}