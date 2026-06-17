#pragma once

#include <Windows.h>
#include <d3d11.h>
#include <wrl/client.h>
#include <DirectXMath.h>


namespace Engine
{
    // DX11 Device / Context / SwapChain / RTV 관리 (Layer 2: RHI)
    class GraphicsRHI
    {
    public:
        GraphicsRHI();
        ~GraphicsRHI();

        bool Initialize(HWND hWnd, int width, int height);
        void Shutdown();

        // 백버퍼를 렌더 타겟으로 묶고 지정 색으로 클리어
        void BeginFrame(float r, float g, float b, float a);        // 화면 클리어
        void DrawTestTriangle();                                    // 삼각형 그리기
        void DrawTestQUad();                                        // 사각형 그리기
        void RenderImGui();                                         // ImGui 그리기 EndFrame() 직전에 호출
        // 백버퍼를 화면에 제출 
        void EndFrame();                                            // 화면에 제출 (Present)

        // 사각형의 월드 위치를 설정 (2-1 : x, y만 회전이랑 스케일은 이후 단계에서 진행)
        void SetQuadPosition(float x, float y);

        ID3D11Device*           GetDevice()  const { return _device.Get(); }
        ID3D11DeviceContext*    GetContext() const { return _context.Get(); }

    private:
        // 삼각형을 그리기 위한 셰이더 / 버퍼 / 레이아웃을 생성 (Initialize에서 호출)
        bool CreateTriangleResources();

        // 사각형을 그리기 위한 셰이더/버퍼/레이아웃 생성 (Initialize에서 호출)
        bool CreateQuadResources();

        // imGui 컨텍스트 + Win32/Dx11 백엔드 초기화
        bool InitImGui(HWND hWnd);

        bool LoadTexture(const std::wstring& path);         // PNG 로딩 -> SRV


        template <typename T>
        using ComPtr = Microsoft::WRL::ComPtr<T>;

        // --- DX11 코어 ---
        ComPtr<ID3D11Device>                _device;            // 자원(버퍼 / 텍스처 / 셰이더) 생성기
        ComPtr<ID3D11DeviceContext>         _context;           // 그리기 명령 기록기
        ComPtr<IDXGISwapChain>              _swapChain;         // 앞뒤 화면 버퍼 교환
        ComPtr<ID3D11RenderTargetView>      _renderTargetView;  // 그릴 대상 (백버퍼)

        // --- Phase 1 삼각형 테스트 리소스 (이후 단계에서 별도 클래스로 분리 예정) ---
        ComPtr<ID3D11VertexShader>          _vertexShader;      // 정점 처리 셰이더
        ComPtr<ID3D11PixelShader>           _pixelShader;       // 픽셀 색 셰이더
        ComPtr<ID3D11InputLayout>           _inputLayout;       // 정점 데이터 구조 명세
        ComPtr<ID3D11Buffer>                _vertexBuffer;      // 꼭짓점 3개 데이터
        ComPtr<ID3D11Buffer>                _indexBuffer;       // 그리는 순서(번호) 데이터
        UINT                                _indexCount = 0;    // 그릴 인덱스 개수 (6)

        ComPtr<ID3D11Buffer>                _transformCB;       // 월드 행렬 상수 버퍼
        DirectX::XMFLOAT2                   _quadPos = { 0.0f, 0.0f };

        // 텍스처 관련
        ComPtr<ID3D11ShaderResourceView>    _textureSRV;       // GPU가 읽을 텍스처
        ComPtr<ID3D11SamplerState>          _samplerState;      // 텍스터 읽는 방식


        bool _imguiInitialized = false;                     // 종료 시 정리 여부 판단용

        int _width = 0;
        int _height = 0;
    };
}