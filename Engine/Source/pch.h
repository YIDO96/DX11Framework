#pragma once

// =========================================================
// 미리 컴파일된 헤더 (Precompiled Header)
// 거의 모든 .cpp가 공통으로 쓰는 무거운 시스템/표준 헤더만 모은다.
// 특정 파일에서만 쓰는 헤더나 라이브러리 pragma는 여기 넣지 않는다.
// =========================================================

// --- Windows ---
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN     // 거의 안 쓰는 Win32 헤더 제외 → 컴파일 가속
#endif
#ifndef NOMINMAX
#define NOMINMAX                // Windows의 min/max 매크로가 std::min/max와 충돌하는 것 방지
#endif
#include <Windows.h>

// --- DirectX 11 ---
#include <d3d11.h>
#include <dxgi.h>
#include <wrl/client.h>         // Microsoft::WRL::ComPtr

// --- C++ 표준 라이브러리 ---
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>


// ImGui (프레임 시작 함수와 UI 위젯 함수를 쓰기 위해)
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"