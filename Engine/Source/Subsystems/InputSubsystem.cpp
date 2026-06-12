#include "pch.h"
#include "InputSubsystem.h"

namespace Engine
{
	void UInputSubsystem::Tick()
	{
		for (int i = 0; i < kKeyCount; ++i)
		{
			_previous[i] = _current[i];
			_current[i] = (GetAsyncKeyState(i) & 0x8000) != 0;
		}
	}


	bool UInputSubsystem::IsKeyDown(int vkey) const
	{
		if (vkey < 0 || vkey >= kKeyCount) return false;
		return _current[vkey];
	}
	bool UInputSubsystem::IsKeyPressed(int vkey) const
	{
		if (vkey < 0 || vkey >= kKeyCount) return false;
		// 지금 눌렸고(현재 true) 직전엔 안 눌림(이전 false) = 막 눌린 순간
		return _current[vkey] && !_previous[vkey];
	}
}