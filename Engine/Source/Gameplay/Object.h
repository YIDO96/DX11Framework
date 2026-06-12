#pragma once

#include <string>

namespace Engine
{
	// 모든 엔진 객체의 최상위 베이스 (언리얼의 UObject와 대응)
	// 지금은 최소한의 뼈대만. 메모리 관리/타입 시스템/핸들은 필요해지는 단계에서 추가.

	class UObject
	{
	public:
		UObject() = default;
		virtual ~UObject() = default;		// 상속 계층이므로 가상 소멸자 필수


		// 디버깅/에디터 표시용 이름
		const std::string& GetName() const { return _name; }
		void SetName(const std::string& name) { _name = name; }

	protected:
		std::string _name = "UObject";
	};
}

