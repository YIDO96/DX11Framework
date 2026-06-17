#include "pch.h"
#include "RHI/GraphicsRHI.h"

#include <d3dcompiler.h>   // 런타임 셰이더 컴파일(D3DCompile)
#include <DirectXTex.h>
#include <cstring>         // strlen

// 이 파일에서만 쓰는 라이브러리를 사용처 근처에 둔다
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")   // ★ 셰이더 런타임 컴파일용 (이번에 추가)

using namespace DirectX;

namespace
{
    // ─────────────────────────────────────────────
    // ① 정점 구조 + 셰이더 소스 (이 .cpp 안에서만 쓰는 것들)
    // ─────────────────────────────────────────────

    // CPU 쪽 정점 한 개의 메모리 배치. 아래 HLSL의 VS_INPUT, 인풋 레이아웃과
    // "순서/크기"가 정확히 일치해야 한다. (position 12바이트 + color 16바이트)
    struct Vertex
    {
        float position[3];   // x, y, z
        //float color[4];      // r, g, b, a
        float uv[2];        // u, v (텍스처 좌표)
    };

    struct TransformCB
    {
        XMMATRIX world;
        //XMFLOAT4X4 world;         // 월드 행렬
    };

    // GPU에서 돌 셰이더 프로그램. HLSL 텍스트를 그대로 문자열로 들고 있다가
    // 실행 시점에 컴파일한다(런타임 컴파일).
    const char* kShaderSource = R"(
cbuffer TransformCB : register(b0)
{
    matrix world;
};

Texture2D       tex     : register(t0);     // 텍스처
SamplerState    samp    : register(s0);     // 샘플러

struct VS_INPUT
{
    float3 position : POSITION;   // 인풋 레이아웃의 "POSITION"과 연결
    float2 uv       : TEXCOORD;
};

struct VS_OUTPUT
{
    float4 position : SV_POSITION; // 화면 좌표(필수 출력)
    float2 uv       : TEXCOORD;
};

    // [Vertex Shader] 꼭짓점 하나하나에 대해 실행. 지금은 위치를 그대로 통과시킴
    // (Phase 2에서 여기에 월드/뷰/투영 행렬 곱셈이 들어간다)
VS_OUTPUT VSMain(VS_INPUT input)
{
    VS_OUTPUT output;
    output.position = mul(float4(input.position, 1.0f), world);
    output.uv       = input.uv;
    return output;
}

    // [Pixel Shader] 삼각형을 채우는 픽셀 하나하나에 대해 실행. 색을 그대로 출력
    // (세 꼭짓점 색이 자동으로 부드럽게 보간되어 무지개처럼 나온다)
float4 PSMain(VS_OUTPUT input) : SV_TARGET
{
    return tex.Sample(samp, input.uv);
}
)";

    // ─────────────────────────────────────────────
    // ② 셰이더 컴파일 헬퍼: HLSL 텍스트 → GPU 바이트코드
    // ─────────────────────────────────────────────
    HRESULT CompileShaderFromString(const char* src, const char* entry,
        const char* target, ID3DBlob** outBlob)
    {
        UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(_DEBUG)
        flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
        Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;
        HRESULT hr = D3DCompile(
            src, std::strlen(src),   // 소스 텍스트와 길이
            nullptr, nullptr, nullptr,
            entry,                   // 진입 함수 이름 (예: "VSMain")
            target,                  // 셰이더 모델 (예: "vs_5_0")
            flags, 0,
            outBlob,                 // 성공 시: 컴파일된 바이트코드
            errorBlob.GetAddressOf());

        // 컴파일 실패 시 에러 메시지를 출력 창에 찍어준다 (디버깅 핵심)
        if (FAILED(hr) && errorBlob)
            OutputDebugStringA(static_cast<const char*>(errorBlob->GetBufferPointer()));

        return hr;
    }
}

namespace Engine
{
    GraphicsRHI::GraphicsRHI() = default;

    GraphicsRHI::~GraphicsRHI()
    {
        Shutdown();
    }

    bool GraphicsRHI::Initialize(HWND hWnd, int width, int height)
    {
        _width = width;
        _height = height;

        // SwapChain 설정을 위한 구조체 만들기
        DXGI_SWAP_CHAIN_DESC scd = {};
        scd.BufferCount = 1;                                    // 백 퍼버(후면 버퍼) 개수 (1이면 전면1 후면1 구조의 더블 버퍼링)
        scd.BufferDesc.Width = width;                           // 백 버퍼 가로 해상도
        scd.BufferDesc.Height = height;                         // 백 버퍼 세로 해상도
        scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;     // 픽셀 포맷 (RGBA 각각 8비트 부호없는 정수 정규화 형식)
        scd.BufferDesc.RefreshRate.Numerator = 60;              // 화면 주사율 분자 (60Hz)
        scd.BufferDesc.RefreshRate.Denominator = 1;             // 화면 주사율 분모
        scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;      // 버퍼 용도 (렌더 타겟 출력용으로 사용)
        scd.OutputWindow = hWnd;                                // 출력할 윈도우 핸들
        scd.SampleDesc.Count = 1;                               // 멀티샘플링 (MSAA) 샘플 개수 (1 = 사용 x)
        scd.SampleDesc.Quality = 0;                             // 멀티 샘플링 품질 수준
        scd.Windowed = TRUE;                                    // 창 모드 여부
        scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;              // 버퍼 교체 효과 (디스플레이 후 이전 프레임 버퍼 내용을 버림)

        UINT flags = 0;
#if defined(_DEBUG)
        flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

        const D3D_FEATURE_LEVEL levels[] = { D3D_FEATURE_LEVEL_11_0 };
        D3D_FEATURE_LEVEL obtainedLevel{};

        HRESULT hr = D3D11CreateDeviceAndSwapChain
        (
            nullptr,                    // 기본 그래픽 어댑터(주 모니터 그래픽 카드) 사용
            D3D_DRIVER_TYPE_HARDWARE,   // 하드웨어 가속(GPU) 사용
            nullptr,                    // 소프트웨어 래스터라이저 모듈 핸들 (하드웨어 모드이므로 null)
            flags,                      // 위에서 설정한 디버그/런타임 플래그
            levels,                     // 지원할 기능 수준(Feature Level) 배열
            _countof(levels),           // 기능 수준 배열 크기
            D3D11_SDK_VERSION,          // 항상 고정으로 들어가는 SDK 버전
            &scd,                       // 앞서 정의한 스왑 체인 설정 구조체
            _swapChain.GetAddressOf(),  // [출력] 생성된 스왑 체인 주소
            _device.GetAddressOf(),     // [출력] 생성된 디바이스 주소
            &obtainedLevel,             // [출력] 최종 선택된 기능 수준
            _context.GetAddressOf()     // [출력] 생성된 디바이스 컨텍스트 주소
        );

#if defined(_DEBUG)
        if (hr == DXGI_ERROR_SDK_COMPONENT_MISSING)
        {
            flags &= ~D3D11_CREATE_DEVICE_DEBUG;
            hr = D3D11CreateDeviceAndSwapChain
            (
                nullptr, 
                D3D_DRIVER_TYPE_HARDWARE, 
                nullptr, 
                flags,
                levels, 
                _countof(levels), 
                D3D11_SDK_VERSION, 
                &scd,
                _swapChain.GetAddressOf(),
                _device.GetAddressOf(),
                &obtainedLevel, _context.GetAddressOf()
            );
        }
#endif
        if (FAILED(hr))
            return false;

        ComPtr<ID3D11Texture2D> backBuffer;
        hr = _swapChain->GetBuffer(0, IID_PPV_ARGS(backBuffer.GetAddressOf()));
        if (FAILED(hr))
            return false;

        hr = _device->CreateRenderTargetView(backBuffer.Get(), nullptr, _renderTargetView.GetAddressOf());
        if (FAILED(hr))
            return false;

        D3D11_VIEWPORT vp = {};
        vp.TopLeftX = 0.0f;
        vp.TopLeftY = 0.0f;
        vp.Width = static_cast<float>(width);
        vp.Height = static_cast<float>(height);
        vp.MinDepth = 0.0f;
        vp.MaxDepth = 1.0f;
        _context->RSSetViewports(1, &vp);

        // ★ DX11 기본 준비가 끝났으니 삼각형 리소스도 생성
        //if (!CreateTriangleResources())
        //    return false;

        if (!CreateQuadResources())
            return false;

        // 텍스처 로딩
        if (!LoadTexture(L"Assets/dog.png"))
            return false;

        // DX11 준비가 완료가 된 이후에 ImGui 초기화
        if (!InitImGui(hWnd))
            return false;

        return true;
    }

    // ─────────────────────────────────────────────
    // ③ 삼각형용 리소스 4종 생성: 셰이더 → 셰이더객체 → 레이아웃 → 정점버퍼
    // ─────────────────────────────────────────────
    bool GraphicsRHI::CreateTriangleResources()
    {
        // (1) HLSL을 컴파일 → 바이트코드(Blob) 획득
        ComPtr<ID3DBlob> vsBlob, psBlob;
        if (FAILED(CompileShaderFromString(kShaderSource, "VSMain", "vs_5_0", vsBlob.GetAddressOf())))
            return false;
        if (FAILED(CompileShaderFromString(kShaderSource, "PSMain", "ps_5_0", psBlob.GetAddressOf())))
            return false;

        // (2) 바이트코드로 실제 셰이더 객체 생성
        if (FAILED(_device->CreateVertexShader(
            vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, _vertexShader.GetAddressOf())))
            return false;
        if (FAILED(_device->CreatePixelShader(
            psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, _pixelShader.GetAddressOf())))
            return false;

        // (3) 인풋 레이아웃: 정점 버퍼의 바이트를 셰이더 입력으로 어떻게 해석할지 명세
        //     - "POSITION": 0번 바이트부터 float 3개
        //     - "COLOR"   : 12번 바이트부터 float 4개
        const D3D11_INPUT_ELEMENT_DESC layout[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        };
        // 인풋 레이아웃은 VS 바이트코드와 대조 검증되므로 vsBlob이 필요하다
        if (FAILED(_device->CreateInputLayout(
            layout, _countof(layout),
            vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(),
            _inputLayout.GetAddressOf())))
            return false;

        // (4) 정점 버퍼: 삼각형 세 꼭짓점. 좌표는 NDC(화면 -1~+1, 중앙이 원점, 위가 +Y)
        const Vertex vertices[] =
        {
            { {  0.0f,  0.5f, 0.0f }, { 1.0f, 0.0f} }, // 꼭대기  - 빨강
            { {  0.5f, -0.5f, 0.0f }, { 0.0f, 1.0f} }, // 우하단  - 초록
            { { -0.5f, -0.5f, 0.0f }, { 0.0f, 0.0f} }, // 좌하단  - 파랑
        };

        D3D11_BUFFER_DESC bd = {};
        bd.Usage = D3D11_USAGE_DEFAULT;
        bd.ByteWidth = sizeof(vertices);
        bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;

        D3D11_SUBRESOURCE_DATA initData = {};
        initData.pSysMem = vertices;

        if (FAILED(_device->CreateBuffer(&bd, &initData, _vertexBuffer.GetAddressOf())))
            return false;

        return true;
    }

    // ─────────────────────────────────────────────
    // ③ 사각형용 리소스: 셰이더/레이아웃은 동일, 정점 4개 + 인덱스 6개 추가
    // ─────────────────────────────────────────────
    bool GraphicsRHI::CreateQuadResources()
    {
        // (1) 셰이더 컴파일
        ComPtr<ID3DBlob> vsBlob, psBlob;
        if (FAILED(CompileShaderFromString(kShaderSource, "VSMain", "vs_5_0", vsBlob.GetAddressOf())))
            return false;
        if (FAILED(CompileShaderFromString(kShaderSource, "PSMain", "ps_5_0", psBlob.GetAddressOf())))
            return false;

        // (2) 셰이더 객체 생성
        if (FAILED(_device->CreateVertexShader(
            vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, _vertexShader.GetAddressOf())))
            return false;
        if (FAILED(_device->CreatePixelShader(
            psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, _pixelShader.GetAddressOf())))
            return false;

        // (3) 인풋 레이아웃 (삼각형 때와 동일)
        const D3D11_INPUT_ELEMENT_DESC layout[] =
        {
            { "POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0,   0, D3D11_INPUT_PER_VERTEX_DATA, 0},
            { "TEXCOORD",  0, DXGI_FORMAT_R32G32_FLOAT,    0,  12, D3D11_INPUT_PER_VERTEX_DATA, 0},
        };

        if (FAILED(_device->CreateInputLayout(
            layout, _countof(layout), vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(),
            _inputLayout.GetAddressOf())))
            return false;

        // 정점: UV 지정 (좌상 0,0 / 우상 1,0 / 좌하 0,1 / 우하 1,1)
        const Vertex vertices[] =
        {
            { {-0.3f,  0.3f, 0.0f}, {0.0f, 0.0f} },// 0 좌상
            { { 0.3f,  0.3f, 0.0f}, {1.0f, 0.0f} },// 1 우상
            { {-0.3f, -0.3f, 0.0f}, {0.0f, 1.0f} },// 2 좌하
            { { 0.3f, -0.3f, 0.0f}, {1.0f, 1.0f} },// 3 우하
        };

        D3D11_BUFFER_DESC vbd = {};
        vbd.Usage           = D3D11_USAGE_DEFAULT;
        vbd.ByteWidth       = sizeof(vertices);
        vbd.BindFlags       = D3D11_BIND_VERTEX_BUFFER;

        D3D11_SUBRESOURCE_DATA vInit = {};
        vInit.pSysMem = vertices;


        if (FAILED(_device->CreateBuffer(&vbd, &vInit, _vertexBuffer.GetAddressOf())))
            return false;

        // (5) 인덱스 버퍼 : 정점을 어떤 순서로 묶어서 삼각형 2개를 만들지
        // 삼각형 A = 0, 1, 2 / B = 2, 1, 3
        const UINT indices[] =
        {
            0, 1, 2,
            2, 1, 3
        };
        _indexCount = _countof(indices);        // 6

        D3D11_BUFFER_DESC ibd = {};
        ibd.Usage           = D3D11_USAGE_DEFAULT;
        ibd.ByteWidth       = sizeof(indices);
        ibd.BindFlags       = D3D11_BIND_INDEX_BUFFER;        // 인덱스 용도로 표시

        D3D11_SUBRESOURCE_DATA iInit = {};
        iInit.pSysMem = indices;

        if (FAILED(_device->CreateBuffer(&ibd, &iInit, _indexBuffer.GetAddressOf())))
            return false;


        // 상수 버퍼 생성
        D3D11_BUFFER_DESC cbd = {};
        cbd.Usage           = D3D11_USAGE_DYNAMIC;
        cbd.ByteWidth       = sizeof(TransformCB);
        cbd.BindFlags       = D3D11_BIND_CONSTANT_BUFFER;
        cbd.CPUAccessFlags  = D3D11_CPU_ACCESS_WRITE;

        if (FAILED(_device->CreateBuffer(&cbd, nullptr, _transformCB.GetAddressOf())))
            return false;


        // 샘플러 상태 생성 (텍스처를 어떻게 읽을지)
        D3D11_SAMPLER_DESC sd = {};
        sd.Filter           = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        sd.AddressU         = D3D11_TEXTURE_ADDRESS_CLAMP;
        sd.AddressV         = D3D11_TEXTURE_ADDRESS_CLAMP;
        sd.AddressW         = D3D11_TEXTURE_ADDRESS_CLAMP;
        sd.ComparisonFunc   = D3D11_COMPARISON_NEVER;
        sd.MinLOD           = 0;
        sd.MaxLOD           = D3D11_FLOAT32_MAX;

        if (FAILED(_device->CreateSamplerState(&sd, _samplerState.GetAddressOf())))
            return false;

        // BlendState 생성
        D3D11_BLEND_DESC bd2 = {};
        bd2.RenderTarget[0].BlendEnable             = TRUE;
        bd2.RenderTarget[0].SrcBlend                = D3D11_BLEND_SRC_ALPHA;            // 새 픽셀 x 알파
        bd2.RenderTarget[0].DestBlend               = D3D11_BLEND_INV_SRC_ALPHA;        // 배경 x (1 - 알파)
        bd2.RenderTarget[0].BlendOp                 = D3D11_BLEND_OP_ADD;               // 둘을 더함
        bd2.RenderTarget[0].SrcBlendAlpha           = D3D11_BLEND_ONE;
        bd2.RenderTarget[0].DestBlendAlpha          = D3D11_BLEND_ZERO;
        bd2.RenderTarget[0].BlendOpAlpha            = D3D11_BLEND_OP_ADD;
        bd2.RenderTarget[0].RenderTargetWriteMask   = D3D11_COLOR_WRITE_ENABLE_ALL;

        if (FAILED(_device->CreateBlendState(&bd2, _blendState.GetAddressOf())))
            return false;

        return true;
    }

    // ─────────────────────────────────────────────
    // ImGui 초기화: 컨텍스트 생성 → Win32 백엔드 → DX11 백엔드
    // ─────────────────────────────────────────────
    bool GraphicsRHI::InitImGui(HWND hWnd)
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;   // 키보드로 ImGui 조작 허용

        ImGui::StyleColorsDark();       // 어두운 테마

        if (!ImGui_ImplWin32_Init(hWnd))
            return false;
        if (!ImGui_ImplDX11_Init(_device.Get(), _context.Get()))
            return false;


        _imguiInitialized = true;
        return true;
    }

    // ─────────────────────────────────────────────
    // DirectXTex로 PNG 로딩 → ShaderResourceView 생성
    // ─────────────────────────────────────────────
    bool GraphicsRHI::LoadTexture(const std::wstring& path)
    {
        ScratchImage image;

        // WIC (Windows Imanging Component)로 PNG/JPG등 디코딩
        HRESULT hr = LoadFromWICFile(path.c_str(), WIC_FLAGS_NONE, nullptr, image);
        if (FAILED(hr))
        {
            OutputDebugStringA("Texture load FAILED: check Assets/dog.png paht\n");
            return false;
        }

        // 디코딩된 이미지로 SRV 생성
        hr = CreateShaderResourceView(
            _device.Get(),
            image.GetImages(),
            image.GetImageCount(),
            image.GetMetadata(),
            _textureSRV.GetAddressOf());

        if (FAILED(hr))
            return false;


        return true;
    }

    void GraphicsRHI::Shutdown()
    {
        // ImGui 정리 (DX11 객체보다 먼저)
        if (_imguiInitialized)
        {
            ImGui_ImplDX11_Shutdown();
            ImGui_ImplWin32_Shutdown();
            ImGui::DestroyContext();
            _imguiInitialized = false;
        }


        if (_context)
            _context->ClearState();

        _indexBuffer.Reset();
        _vertexBuffer.Reset();
        _inputLayout.Reset();
        _pixelShader.Reset();
        _vertexShader.Reset();

        _blendState.Reset();
        _samplerState.Reset();
        _textureSRV.Reset();

        _renderTargetView.Reset();
        _swapChain.Reset();
        _context.Reset();
        _device.Reset();
    }

    void GraphicsRHI::BeginFrame(float r, float g, float b, float a)
    {
        ID3D11RenderTargetView* rtv = _renderTargetView.Get();
        _context->OMSetRenderTargets(1, &rtv, nullptr);

        const float color[4] = { r, g, b, a };
        _context->ClearRenderTargetView(_renderTargetView.Get(), color);
    }

    // ─────────────────────────────────────────────
    // ④ 매 프레임: 만들어둔 리소스를 GPU에 "장착"하고 그리기 명령
    // ─────────────────────────────────────────────
    void GraphicsRHI::DrawTestTriangle()
    {
        UINT stride = sizeof(Vertex);   // 정점 1개 크기(28바이트)
        UINT offset = 0;

        // IA(Input Assembler) 단계: 무엇을, 어떤 형식으로, 어떻게 묶어 읽을지
        _context->IASetInputLayout(_inputLayout.Get());
        _context->IASetVertexBuffers(0, 1, _vertexBuffer.GetAddressOf(), &stride, &offset);
        _context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST); // 정점 3개 = 삼각형

        // 셰이더 단계: 어떤 VS/PS로 처리할지
        _context->VSSetShader(_vertexShader.Get(), nullptr, 0);
        _context->PSSetShader(_pixelShader.Get(), nullptr, 0);

        // 그려라! (정점 3개를 0번부터)
        _context->Draw(3, 0);
    }
    void GraphicsRHI::SetQuadPosition(float x, float y)
    {
        _quadPos = { x, y };
    }
    void GraphicsRHI::DrawTestQuad()
    {
        // 현재 위치로 월드 행렬 만들기
        // 행렬은 셰이더에서 row-majon로 곱하기에 Transpose해서 보낸다.
        XMMATRIX world = XMMatrixTranslation(_quadPos.x, _quadPos.y, 0.0f);

        // 상수 버퍼에 행렬 써넣기
        D3D11_MAPPED_SUBRESOURCE mapped{};
        if (SUCCEEDED(_context->Map(_transformCB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)))
        {
            TransformCB* cb = reinterpret_cast<TransformCB*>(mapped.pData);
            cb->world = XMMatrixTranspose(world);
            //XMStoreFloat4x4(&cb->world, XMMatrixTranspose(world));
            _context->Unmap(_transformCB.Get(), 0);
        }


        UINT stride = sizeof(Vertex);
        UINT offset = 0;

        _context->IASetInputLayout(_inputLayout.Get());
        _context->IASetVertexBuffers(0, 1, _vertexBuffer.GetAddressOf(), &stride, &offset);
        // 인덱스 버퍼 장착 (UINT 인덱스이므로 R32_UINT)
        _context->IASetIndexBuffer(_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
        _context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        _context->VSSetShader(_vertexShader.Get(), nullptr, 0);
        _context->VSSetConstantBuffers(0, 1, _transformCB.GetAddressOf());

        _context->PSSetShader(_pixelShader.Get(), nullptr, 0);

        // 텍스처와 샘플러를 픽셀 셰이더에 바인딩
        _context->PSSetShaderResources(0, 1, _textureSRV.GetAddressOf());
        _context->PSSetSamplers(0, 1, _samplerState.GetAddressOf());

        // 블렌드 스테이트 적용
        const float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
        _context->OMSetBlendState(_blendState.Get(), blendFactor, 0xffffffff);


        TransformCB* cb = reinterpret_cast<TransformCB*>(mapped.pData);
        cb->world = XMMatrixTranspose(world);
        // Draw -> DrawIndexed 로 변경 (인덱스 6개를 0번부터)
        _context->DrawIndexed(_indexCount, 0, 0);
    }

    void GraphicsRHI::RenderImGui()
    {
        ImGui::Render();
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    }

    void GraphicsRHI::EndFrame()
    {
        _swapChain->Present(1, 0);   // 1 = VSync ON
    }
    
}