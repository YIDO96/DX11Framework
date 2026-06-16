#pragma once

namespace Engine
{
	// 키보드/마우스 입력 상태를 제공 (Layer 3: Subsystem)
	// 2-3 : GetAsyncKeyState 기반 폴링, "지금 이 키가 눌려있는가"를 묻는 단순한 방식
	class UInputSubsystem
	{
	public:
		// 매 프레임 시작 시 1회 호출. 이번 프레임의 키 상태 스탭샷을 갱신
		void Tick();

		// 지정한 키가 현재 눌려있는지
		bool IsKeyDown(int vkey) const;

		// 이번 프레임에 막 눌린 순간인가
		bool IsKeyPressed(int vkey) const;

	private:
		static constexpr int kKeyCount = 256;
		bool _current[kKeyCount] = {};			// 이번 프레임 눌림 상태
		bool _previous[kKeyCount] = {};			// 직전 프레임 눌림 상태
	};

}