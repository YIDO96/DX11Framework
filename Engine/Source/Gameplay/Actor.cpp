#include "pch.h"
#include "Actor.h"

namespace Engine
{
	AActor::AActor()
	{
		_name = "Actor";

		// 액터 생성과 동시에 RootComponent를 가진다 -> 비로소 월드 공간에 존재
		_rootComponent = std::make_unique<USceneComponent2D>();
	}
}