#include "pch.h"
#include "Core/Engine.h"

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
    Engine::EngineApp app;

    if (!app.Initialize(hInstance))
    {
        MessageBoxW(nullptr, L"엔진 초기화에 실패했습니다.", L"오류", MB_OK | MB_ICONERROR);
        return -1;
    }

    app.Run();
    app.Shutdown();
    return 0;
}