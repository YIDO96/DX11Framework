#include "pch.h"
#include "Core/Engine.h"
#include "Core/Window.h"
#include "RHI/GraphicsRHI.h"
#include "Subsystems/TimeSubsystem.h"
#include "Subsystems/InputSubsystem.h"
#include "Gameplay/TestMovingActor.h"

#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"

namespace Engine
{
    // unique_ptr<전방선언 타입> 때문에 소멸자는 반드시 .cpp에서 정의(헤더 포함된 곳)
    EngineApp::EngineApp() = default;
    EngineApp::~EngineApp() = default;

    bool EngineApp::Initialize(HINSTANCE hInstance)
    {
        _window = std::make_unique<Window>();
        if (!_window->Initialize(hInstance, 1280, 720, L"DeckCrawler - Phase 1"))
            return false;

        _graphics = std::make_unique<GraphicsRHI>();
        if (!_graphics->Initialize(_window->GetHandle(), _window->GetWidth(), _window->GetHeight()))
            return false;

        _time = std::make_unique<TimeSubsystem>();
        _input = std::make_unique<UInputSubsystem>();


        // 액터 생성 + 초기 위치 + BeginPlay
        auto player = std::make_unique<TestMovingActor>();
        player->SetPosition(0.0f, 0.0f);
        player->SetInput(_input.get());
        player->BeginPlay();
        _player = std::move(player);


        _isRunning = true;
        return true;
    }

    void EngineApp::Run()
    {
        while (_isRunning)
        {
            if (!_window->ProcessMessages())   // WM_QUIT이면 false
            {
                _isRunning = false;
                break;
            }

            Tick();
        }
    }

    void EngineApp::Tick()
    {
        _input->Tick();
        // 시간 갱신
        _time->Tick();
        const float dt = _time->GetDeltaTime();

        // 액터의 Tick
        _player->Tick(dt);

        // 액터의 현재 위치를 렌더러에 전달
        const DirectX::XMFLOAT2& pos = _player->GetPosition();
        _graphics->SetQuadPosition(pos.x, pos.y);

        //// 사각형 이동
        //_quadX += _quadVelX * dt;
        //
        //// 화면 양끝에 도착하면 방향 반전 ->좌우 반복
        //if (_quadX > 0.7f) { _quadX = 0.7f; _quadVelX = -_quadVelX; }
        //if (_quadX < -0.7f) { _quadX = -0.7f; _quadVelX = -_quadVelX; }
        //
        //_graphics->SetQuadPosition(_quadX, 0.f);

        // 1) ImGui 새 프레임 시작 (입력 상태 갱신)
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        // 2) UI 구성 - 우선은 FPS / 프레임타임을 보여주는 디버그 창 하나
        {
            ImGui::Begin("Debug");
            ImGui::Text("DeckCrawler - Phase 1");
            ImGui::Text("FPS : %.1f (%.3f ms/frame)", ImGui::GetIO().Framerate, 1000.f / ImGui::GetIO().Framerate);
            ImGui::Separator();
            ImGui::Text("Actor : %s", _player->GetName().c_str());
            ImGui::Text("Position : (%.3f, %.3f)", pos.x, pos.y);
            ImGui::Text("DeltaTime : %.4f s", dt);
            ImGui::End();

            // ImGui 기능을 한눈에 보고 싶으면 데모 창을 켜보세요 (학습용)
            static bool showDemo = false;
            if (showDemo) ImGui::ShowDemoWindow(&showDemo);
        }


        _graphics->BeginFrame(0.1f, 0.2f, 0.4f, 1.0f);   // 짙은 파랑으로 클리어
        // 이후 Phase에서 이 사이에 렌더링 로직이 들어감
        //_graphics->DrawTestTriangle();                 // 2) 그 위에 삼각형을 그린 뒤
        _graphics->DrawTestQUad();

        _graphics->RenderImGui();
        _graphics->EndFrame();
    }

    void EngineApp::Shutdown()
    {
        _player.reset();
        _input.reset();
        _time.reset();

        if (_graphics)
        {
            _graphics->Shutdown();
            _graphics.reset();
        }
        if (_window)
        {
            _window->Shutdown();
            _window.reset();
        }
        _isRunning = false;
    }
}