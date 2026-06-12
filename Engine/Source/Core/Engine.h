#pragma once

#include <Windows.h>
#include <memory>

namespace Engine
{
    class Window;
    class GraphicsRHI;
    class TimeSubsystem;

    // 프레임 라이프사이클 총괄 구동 (Layer 1: Core)
    class EngineApp
    {
    public:
        EngineApp();
        ~EngineApp();

        bool Initialize(HINSTANCE hInstance);
        void Run();
        void Shutdown();

    private:
        void Tick();

    private:
        std::unique_ptr<Window>         _window;
        std::unique_ptr<GraphicsRHI>    _graphics;
        std::unique_ptr<TimeSubsystem>  _time;
        bool _isRunning = false;

        float _quadX = 0.f;
        float _quadVelX = 0.4f;
    };
}