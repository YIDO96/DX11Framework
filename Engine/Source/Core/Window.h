#pragma once

#include <Windows.h>
#include <string>

namespace Engine
{
    // Win32 HWND 생성 및 메시지 루프 캡슐화 (Layer 1: Core)
    class Window
    {
    public:
        Window();
        ~Window();

        bool Initialize(HINSTANCE hInstance, int width, int height, const std::wstring& title);
        void Shutdown();

        // 대기 중인 메시지를 모두 처리. 종료(WM_QUIT) 요청 시 false 반환.
        bool ProcessMessages();

        HWND GetHandle() const { return m_hWnd; }
        int  GetWidth()  const { return m_width; }
        int  GetHeight() const { return m_height; }

    private:
        static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
        LRESULT HandleMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

    private:
        HWND         m_hWnd = nullptr;
        HINSTANCE    m_hInstance = nullptr;
        int          m_width = 0;
        int          m_height = 0;
        std::wstring m_className = L"DeckCrawlerWindowClass";
    };
}