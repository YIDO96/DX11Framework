#include "pch.h"
#include "TimeSubsystem.h"


namespace Engine
{
	TimeSubsystem::TimeSubsystem()
	{
		// 타이머 주파수 (초당 카운트)와 시작 시점을 기록
		LARGE_INTEGER freq{};
		QueryPerformanceFrequency(&freq);
		_frequency = freq.QuadPart; // 초당 경과

		LARGE_INTEGER now{};
		QueryPerformanceCounter(&now);
		_prevCount = now.QuadPart; // 시작 지점  (직전 카운트)
	}

	void TimeSubsystem::Tick()
	{
		LARGE_INTEGER now{};
		QueryPerformanceCounter(&now);


		// 현재 카운트 - 직전 카운트 / 초당 카운트 = 경과 초
		const int64_t delta = now.QuadPart - _prevCount;
		_deltaTime = static_cast<float>(delta) / static_cast<float>(_frequency);

		
		// 비정삭적으로 큰 값은 방지 (디버거에서 멈췄다가 재개하게 되면 delta값이 폭증할 수 있다)
		if (_deltaTime > 0.1f)
			_deltaTime = 0.1f;

		_totalTime += _deltaTime;
	}
}