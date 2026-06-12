#include "pch.h"
#include "Core/Window.h"

#include "imgui_impl_win32.h"

// ImGui win32 백엔드가 제공하는 메시지 핸들러의 전방 선언
// 헤더에도 있지만, 명시적으로 선언해두는 게 ImGui 공식 예제의 관례
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace Engine
{
    Window::Window() = default;

    Window::~Window()
    {
        Shutdown();
    }

    bool Window::Initialize(HINSTANCE hInstance, int width, int height, const std::wstring& title)
    {
        m_hInstance = hInstance;
        m_width = width;
        m_height = height;

        WNDCLASSEXW wc = {};
        wc.cbSize = sizeof(WNDCLASSEXW);
        wc.style = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc = &Window::WndProc;
        wc.hInstance = hInstance;
        wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
        wc.lpszClassName = m_className.c_str();

        if (!RegisterClassExW(&wc))
            return false;

        const DWORD style = WS_OVERLAPPEDWINDOW;

        // 클라이언트 영역이 정확히 width x height가 되도록 창 전체 크기 보정
        RECT rect = { 0, 0, width, height };
        AdjustWindowRect(&rect, style, FALSE);
        const int winWidth = rect.right - rect.left;
        const int winHeight = rect.bottom - rect.top;

        m_hWnd = CreateWindowExW(
            0,
            m_className.c_str(),
            title.c_str(),
            style,
            CW_USEDEFAULT, CW_USEDEFAULT,
            winWidth, winHeight,
            nullptr, nullptr,
            hInstance,
            this);                  // lpParam → WM_NCCREATE에서 인스턴스 포인터로 수신

        if (!m_hWnd)
            return false;

        ShowWindow(m_hWnd, SW_SHOW);
        UpdateWindow(m_hWnd);
        return true;
    }

    void Window::Shutdown()
    {
        if (m_hWnd)
        {
            DestroyWindow(m_hWnd);
            m_hWnd = nullptr;
        }
        if (m_hInstance)
        {
            UnregisterClassW(m_className.c_str(), m_hInstance);
            m_hInstance = nullptr;
        }
    }

    bool Window::ProcessMessages()
    {
        MSG msg = {};
        while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
                return false;

            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
        return true;
    }

    LRESULT CALLBACK Window::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        // ImGui에게 메시지를 먼저 전달. imGui가 처리했다면(1 반환) 여기서 끝
        // (ImGui 창 위세어의 마우스 키보드 입력이 게임으로 새지 않도록
        if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
            return 1;

        // 창 생성 시 CreateWindowEx의 lpParam(this)을 받아 USERDATA에 저장
        if (msg == WM_NCCREATE)
        {
            const CREATESTRUCTW* cs = reinterpret_cast<CREATESTRUCTW*>(lParam);
            Window* self = static_cast<Window*>(cs->lpCreateParams);
            SetWindowLongPtrW(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
            self->m_hWnd = hWnd;
        }

        Window* self = reinterpret_cast<Window*>(GetWindowLongPtrW(hWnd, GWLP_USERDATA));
        if (self)
            return self->HandleMessage(hWnd, msg, wParam, lParam);

        return DefWindowProcW(hWnd, msg, wParam, lParam);
    }

    LRESULT Window::HandleMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        switch (msg)
        {
        case WM_CLOSE:
            DestroyWindow(hWnd);
            return 0;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        }
        return DefWindowProcW(hWnd, msg, wParam, lParam);
    }
}