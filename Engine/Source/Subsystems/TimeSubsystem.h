#pragma once

#include <cstdint>

namespace Engine
{
	// 프레임 간 경과 시간 (DeltaTime)을 측정
	// 고해상도 타이머 (QueryPerformanceCounter)를 사용해서 정밀하게 잰다

	class TimeSubsystem
	{
	public:
		TimeSubsystem();

		// 매 프레임 시작 시 1회 호출. 직전 Tick 이후 흐른 시간을 갱신
		void Tick();

		// 직전 프레임 소요 시간 (초 단위) 예 : 60FPS면 약 0.0166
		float GetDeltaTime() const { return _deltaTime; }

		// 프로그램 시작 후 총 경과 시간 (초)
		float GetTotalTime() const { return _totalTime; }
	private:
		int64_t _frequency = 0;
		int64_t _prevCount = 0;
		float _deltaTime = 0.f;
		float _totalTime = 0.f;
	};
}