# 🧟 좀보이드식 로드맵 (v3) — Phase 4 이후 재구성

> **게임 방향:** 탑다운 2D 실시간 생존 (좀보이드식)
> **렌더 뷰:** 순수 탑다운으로 완성 → 아이소메트릭/레이캐스팅은 후순위 학습 렌더 경로
> **전제:** 게임 로직은 격자(논리) 좌표로, 화면 변환은 GridSystem 한 곳에 격리 (Phase 3에서 완료)

---

## ✅ 완료 (Phase 1~3)

- **Phase 1** — 창 + DX11 파이프라인 + ImGui + 삼각형/사각형
- **Phase 2** — 시간(DeltaTime) + 상수버퍼/월드행렬 + UObject/AActor/SceneComponent + 입력
- **Phase 3** — 텍스처(DirectXTex) + UV + 알파블렌딩 + SpriteComponent(제출 방식) + 격자 좌표계(GridSystem)

핵심 자산: **제출(submit) 렌더 방식 + 격자 좌표계.** 이 둘 덕분에 "다수 액터를 격자 위에 배치"가 가능해졌고, 이게 좀보이드식의 토대.

---

## 🎞️ Phase 4 — 스프라이트 애니메이션

> 변화 거의 없음. 좀보이드식에도 그대로 필요(걷기/공격 애니메이션).

- 스프라이트 시트에서 UV Offset 연산으로 프레임 전환
- 언리얼 Flipbook 개념의 `UFlipbookComponent2D` (PlayRate, Loop, 현재 프레임)
- ImGui로 재생/일시정지/프레임 스텝 디버깅
- **좀보이드식 추가 고려:** 4방향(또는 8방향) 이동 애니메이션 — 이동 방향에 따라 다른 행(row) 재생. 단, 지금은 단일 애니메이션으로 시작하고 방향별은 나중에.

---

## 🎮 Phase 5 — Pawn / Controller / 카메라

> 그대로 필요. **여기가 "엔진 제작 → 게임 제작" 분기점.**

- 입력 전담 `APlayerController` + `APawn` 빙의(Possess) 구조
- `UCameraComponent` — 플레이어를 따라다니는 카메라
- **투영 행렬 정식화** — Phase 3에서 임시로 둔 `GridSystem::WorldToNDC`를 카메라의 View/Projection 행렬로 교체. 카메라가 움직이면 월드가 스크롤됨
- **★ PlayerSquare를 Game 프로젝트의 정식 Pawn으로 이전** — 테스트용 `PlayerSquare` 삭제, Game에 `APlayerPawn` 신설. Engine은 게임 객체를 모르는 상태로 정리
- ImGui 카메라 디버그 패널

> 카메라가 들어오면 격자→화면 변환이 카메라 기준으로 바뀌므로, 큰 맵에서 플레이어 주변만 보이는 구조가 완성됨. 좀보이드식 필수.

---

## 🗺️ Phase 6 — 맵 생성 + 타일맵 렌더링

> 덱빌딩엔 없던 큰 단계. 좀보이드식의 월드가 여기서 생김.

- **펄린노이즈 맵 생성** — 시드 기반으로 격자 맵 데이터(바닥/벽 등) 생성. 시드 하나로 동일 맵 재현(세이브/멀티 확장성)
- **타일맵 렌더링** — 격자 칸마다 `GridToScreen`으로 위치 잡아 타일 스프라이트 제출. (Phase 3 구조 위에서 거의 공짜)
- **카메라 컬링** — 화면에 보이는 칸만 그리기 (맵이 크므로 전체를 매번 그리면 느림)
- ImGui로 시드 변경 / 맵 재생성 테스트

> 타일맵이 여기서 나오는 이유: 격자 좌표계(Phase 3) × 맵 데이터(여기) 둘이 만나야 성립.

---

## 🧱 Phase 7 — World + 다수 액터 관리

> 원래 Phase 7(World)이지만 좀보이드식이라 **충돌·다수 AI 직전에 필수**가 됨.

- **`UWorld` / `ULevel`** — 액터를 배열로 소유·관리. `SpawnActor` / `DestroyActor`
- **소유권 이전** — `EngineApp`이 직접 들던 액터를 World로 이전 (Phase 2B에서 예고한 그 작업)
- **★ 핸들/수명 시스템(PendingKill)** — 좀비가 죽어 파괴될 때 다른 액터가 죽은 좀비를 참조하면 터지므로, 약참조 핸들 + IsValid 도입
- 매 프레임 `World → Level → Actors` 순회 Tick (v2 라이프사이클 정식화)

> 좀비가 수십 마리 스폰/파괴되므로 World·수명 관리가 덱빌딩보다 일찍·무겁게 필요.

---

## 💥 Phase 8 — 충돌 + 델리게이트

> 좀보이드식 실시간 전투의 핵심 토대.

- **`UBoxComponent2D`** (AABB 충돌) — 격자 기반이라 단순화 가능
- **멀티캐스트 델리게이트** 정식 도입 — `OnBeginOverlap`, `OnHit` 등. (v2에서 "충돌 때 처음 만들지 말라"고 했지만, 좀보이드식은 입력 액션이 Phase 5에 적으므로 여기서 만들어도 무방)
- 좀비-플레이어, 좀비-벽, 공격 판정이 매 프레임 실시간으로 처리
- ImGui 충돌 디버그 드로우 On/Off

---

## 🧠 Phase 9 — 좀비 AI + 추적

> 덱빌딩의 "전투 트랙"을 대체하는 실시간 AI 단계.

- **`AIController` + FSM** — Idle / 추적(Chase) / 공격(Attack) 상태
- **A* 추적** — 격자 위 경로탐색. ⚠️ 계층적 길찾기는 측정 후 최적화(미리 X)
- **시야/거리 감지** — 플레이어가 시야·범위에 들어오면 추적 시작
- 다수 좀비 동시 AI — 부하 생기면 그때 최적화(틱 분산 등)

---

## 🎒 Phase 10 — 생존 시스템 (게임플레이 본체)

> 좀보이드식 "게임"이 완성되는 단계.

- 플레이어 스탯(체력 등), 좀비 데미지/사망
- 아이템 / 인벤토리 / 근접·원거리 공격
- 게임오버 / 리스폰
- **`AGameModeBase`** — 게임 규칙, 스폰 관리, 승패 조건
- ImGui QA 치트(무적, 좀비 즉사, 아이템 지급 등)

---

## 🌄 후순위 (스트레치) — 학습용 렌더 경로

> 게임 본편(탑다운)과 분리. 게임 디자인에 영향 없음.

- **아이소메트릭 렌더 경로** — 같은 격자를 마름모로 그리기. `GridSystem::GridToWorld`만 교체하면 됨. "좀보이드처럼 보이게" 비주얼 업그레이드
- **레이캐스팅 1인칭 뷰** — 같은 격자를 둠 방식으로 그리기. 순수 학습/기술 데모용. DDA 최적화, 몬스터 빌보드
- **마우스 드래그앤드롭 / 정교한 UI**

---

## 📌 덱빌딩 v2 대비 핵심 변경

| 항목 | 덱빌딩 v2 | 좀보이드 v3 |
|---|---|---|
| 전투 | 턴제 카드(커맨드/FSM/CSV) | **실시간 좀비 전투** |
| World | Phase 7 | **Phase 7 (충돌 직전 필수로 앞당김)** |
| 맵 생성 | 후반 | **Phase 6 (펄린노이즈)** |
| 다수 AI | 적음(부하 무관) | **Phase 9 (A* + FSM, 부하 고려)** |
| 핵심 재미 | 카드 조합 | **실시간 생존** |
| 데이터 테이블(CSV) | 카드 정의용 필수 | 선택(아이템 정의 등에 활용 가능) |

---

## ⚠️ 진행 원칙 (계속 유지)

1. **Engine은 장르를 모른다** — "Zombie", "Item", "Survival"은 Game 프로젝트에. Engine엔 AActor/Grid/RHI 같은 토대만.
2. **지금 필요한 것만** — 미리 추상화 금지(rule of three). 부하·확장은 측정/필요 후 대응.
3. **격자 좌표로 로직, 변환은 한 곳에** — 아이소메트릭/레이캐스팅/카메라 전환이 GridSystem 교체로 끝나도록.
4. **단계마다 눈에 보이는 결과 + 빌드 확인** 후 다음으로.




# DX112DFramework

언리얼 프레임워크

# 📑 Unreal Style DirectX11 2D Engine & Framework 설계 (v2)

> **대상 게임:** 그리드 던전 크롤러(실시간 탐험) + 턴제 덱빌딩 전투 하이브리드 or 좀보이드식
**렌더링:** 2D 탑다운을 기본으로 완성하고, 레이캐스팅 1인칭 뷰는 스트레치 골(보너스)
> 

---

## 🚀 1. 프로젝트 개요

- **목표:** 상용 엔진 없이 Win32 API + DirectX11로 밑바닥부터 2D 게임 엔진 프레임워크를 구축한다.
- **핵심 아키텍처:** 유니티식 평평한 컴포넌트 구조가 아닌, 언리얼 스타일의 클래스 상속 구조와 게임플레이 프레임워크 규칙을 이식한다.
- **프로젝트 분리**
    - **Engine 프로젝트** (Static Library) : 저수준 그래픽스(RHI), 컴포넌트 시스템, 전역 매니저(Subsystem), 게임플레이 프레임워크(Actor/Pawn/...) 등 **장르를 모르는** 엔진 코어
    - **Game   프로젝트** (Application)    : 엔진을 링크하여 콘텐츠(탐험 액터, 전투 시스템, 카드 데이터, 에셋)만 작성

---

## 🧭 2. 최상위 설계 원칙 — 의존성 방향 (가장 중요)

엔진의 확장성은 이 한 가지 규칙으로 결정된다.

```cpp
[ Engine.lib ]   ← "Card", "Deck", "Turn" 이라는 단어를 절대 포함하지 않는다.
      ▲              절대 위 계층을 참조하지 않는다.
      │  (Game → Engine 단방향 참조. 역방향 금지)
      │
[ Game (Gameplay + Content) ]   ← 카드 / 덱 / 턴 / 전투 / 탐험 콘텐츠는 전부 여기
```

- **Engine은 "스프라이트를 그린다 / 텍스처를 캐싱한다 / 마우스가 눌렸다"까지만 안다.** 카드·턴·덱은 모른다.
- 이 규칙만 지키면 나중에 다른 2D 게임(플랫포머 등)을 만들 때 Engine을 그대로 재사용할 수 있다.
- **과한 범용화 금지:** 두 번째 사용처가 나타나기 전까지 미리 추상화하지 않는다(rule of three). 레이어만 분리돼 있으면 나중 리팩터링 비용은 낮다.

---

## 🏗️ 3. 클래스 계층 구조 (Unreal Style)

```cpp
[ UObject ]  (최상위 베이스: 메모리 관리, 타입 시스템, 핸들/수명 관리)
   │
   ├── [ UActorComponent ]  (논리 컴포넌트)
   │      └── [ USceneComponent2D ]  (Transform: 위치/회전/크기 소유)
   │             ├── [ USpriteComponent ]      (2D 스프라이트 렌더러)
   │             ├── [ UFlipbookComponent2D ]  (스프라이트 시트 애니메이션)
   │             ├── [ UCameraComponent ]      (View 행렬 제공)
   │             └── [ UPrimitiveComponent2D ] (충돌/렌더 기반)
   │                    └── [ UBoxComponent2D ] (AABB 충돌체)
   │
   └── [ AActor ]  (월드 스폰 가능 최상위 객체. RootComponent 소유)
          ├── [ AGameModeBase ]  (게임 규칙 제어, 스폰 관리)
          ├── [ AController ]    (Pawn을 조종하는 뇌)
          │      ├── [ APlayerController ]  (키보드/마우스 입력 처리)
          │      └── [ AIController ]       (몬스터 AI FSM 처리)
          └── [ APawn ]  (빙의될 수 있는 액터)
                 └── [ ACharacter2D ]  (그리드 이동 로직 특화 Pawn → 플레이어/몬스터)
```

### 💡 핵심 포인트

1. **Actor vs Component:** `AActor`는 월드 좌표를 직접 갖지 않는다. 내부의 `RootComponent`(USceneComponent2D)를 통해 비로소 월드 공간에 존재한다.
2. **Controller vs Pawn:** 캐릭터(`ACharacter2D`) 안에 입력 처리가 들어가지 않는다. 입력 해석은 `APlayerController`가 전담하고, 조종 대상(`APawn`)에게 명령을 내린다.
3. **⚠️ 전투는 이 계층에 없다:** 턴제 카드 전투는 `Actor::Tick(DeltaTime)`으로 돌아가지 않는다. **커맨드 패턴 + 상태 머신 + 데이터 테이블**로 돌아가는 별도 시스템이며, Game 프로젝트 상층에 위치한다(§7 참고). Actor 프레임워크에 억지로 끼워넣지 않는다.

---

## 📦 4. 엔진 레이어 설계 (Layered Architecture)

### 🔹 Layer 4 : Gameplay Framework

언리얼 아키텍처 규칙 클래스군 (`UObject`, `AActor`, `APawn`, `AController`, `UWorld`, `ULevel`, `AGameModeBase`)

### 🔹 Layer 3 : Engine Subsystems (전역 관리자)

- **`UInputSubsystem`** : 키보드/마우스 상태 폴링, 축(Axis)/액션(Action) 매핑
- **`UResourceSubsystem`** : 파일 경로를 키로 텍스처(SRV)/셰이더/메쉬를 중복 없이 캐싱하는 플라이웨이트 매니저
- **`UTimeSubsystem`** : 정밀 타이머 기반 가변 DeltaTime + (필요 시) 고정 타임스텝 계측
- **🆕 `Delegate` (멀티캐스트 이벤트 시스템)** : `OnClicked`, `OnTurnStart`, `OnBeginOverlap`, 카드 효과 등 이벤트는 전방위로 쓰이므로 **코어에 미리** 둔다. 충돌(Phase 6)에서 처음 만들지 않는다.
- **🆕 Object 수명/핸들 관리** : 파괴 마킹된 액터를 다른 액터가 포인터로 들고 있으면 댕글링으로 터진다. **약참조 핸들 + `IsValid()`/`PendingKill` 체크**를 UObject 단계에서 함께 설계한다. (deferred destroy 안전성)

### 🔹 Layer 2 : RHI (Render Hardware Interface) — **두 렌더 모드가 공유**

- **`GraphicsRHI`** : DX11 Device, Context, SwapChain, RenderTargetView 관리
- **`DX11Shader`** : Vertex/Pixel Shader + Input Layout 캡슐화
- **`DX11State`** : BlendState(알파), SamplerState(필터링), RasterizerState 등 파이프라인 상태
- **`DX11Buffer` (상수 버퍼)** : 용도별로 엄격히 분리, **16바이트 정렬 준수**

| 상수 버퍼 | 레지스터 슬롯 | 갱신 주기 | 주요 데이터 |
| --- | --- | --- | --- |
| `CB_ViewProj` | `b0` | 프레임당 1회 | `MatrixView`, `MatrixProjection` (카메라 이동/줌) |
| `CB_ActorTransform` | `b1` | 드로우당 1회 | `MatrixWorld` (오브젝트별 위치/회전/크기) |
| `CB_Material` | `b2` | 소재 변경 시 | `ColorTint`, `UV Offset/Scale` (애니메이션 좌표) |

> **🆕 슬롯 고정 규칙:** 모든 셰이더에서 레지스터 번호(b0/b1/b2)를 동일하게 고정한다. 슬롯 충돌은 디버깅이 매우 괴롭다.
**🆕 배칭 정책 결정:** 본 프로젝트 규모(스프라이트 수백 개)에서는 **배칭하지 않고 "드로우당 CB 갱신"으로 정직하게 간다.** (진짜 배칭은 트랜스폼을 인스턴싱/StructuredBuffer로 빼야 하므로 지금은 과잉.)
> 

### 🔹 Layer 1 : Core Engine App

- **`Engine`** : 메인 루프 / 프레임 라이프사이클 총괄 구동
- **`Window`** : Win32 HWND 생성 및 PeekMessage 메시지 프로시저 캡슐화

---

## 🎨 5. 좌표계 & 렌더링 규약 (🆕 명시 필수)

- **투영:** 직교(Orthographic). 화면 = 월드 단위.
- **좌표 원점 / 축:** (예) 좌상단 원점, X→오른쪽, Y→아래 (또는 중앙 원점) — **프로젝트 시작 시 하나로 고정**
- **Pixels-Per-Unit:** 1유닛 = N픽셀 기준 명시
- **🆕 드로우 순서 / 정렬:** 알파 블렌딩을 켜는 순간(Phase 3) 반투명은 **back-to-front**로 그려야 올바르다. UI / 보드 / 카드 / 이펙트가 겹치므로 **sort key(레이어 + Y 또는 명시적 Z) 기반 정렬**을 Phase 3에 함께 도입한다. (나중에 끼워넣기 까다로움)

---

## 🖼️ 6. 렌더링 패러다임 — 두 개의 뷰

```cpp
[ RHI : 텍스처 쿼드 드로우 ]   ← 두 모드가 공유. 장르 무관.
      ▲                      ▲
      │                      │
[ 탑다운 2D 렌더 ]      [ 레이캐스트 렌더 ]   ← 교체 가능한 뷰 (스트레치)
      └──────────┬────────────┘
                 │  같은 그리드 맵 / 같은 시뮬레이션을 그릴 뿐
```

- *시뮬레이션(이동, A 추적, FSM 전투 전환)은 렌더 모드와 무관하게 동일하다.*렌더링은 같은 시뮬레이션을 보여주는 "뷰 선택"일 뿐.
- 레이캐스팅의 벽 스트립도 결국 "텍스처 입힌 세로 쿼드", 몬스터 빌보드도 "스크린 공간 스프라이트"라 **Layer 2 RHI를 공유**한다. 달라지는 건 그 위층뿐.

---

## 🗺️ 7. 단계별 개발 로드맵 (재정렬)

> **핵심 전략 2가지**
> 
> 1. **레이캐스팅(Phase 9)은 진짜 보너스로 둔다.** 탑다운 2D만으로도 게임이 완성되게 설계 → 레이캐스팅 못 끝내도 게임은 완성됨.
> 2. **가장 먼저 깊이 고민할 건 전투(§Phase 8).** 가장 정의가 안 됐고 레버리지가 가장 크다.

### 📌 [Phase 1] 파이프라인 기초 — 화면에 사각형

- **결과:** 창이 켜지고 단색 사각형 + FPS 미터 + ImGui 디버그 창 출력
- Win32 윈도우 생성 + PeekMessage 메인 루프
- DX11 핵심 객체(Device, Context, SwapChain, RTV) 초기화
- ImGui 초기화 및 WinProc 입력 후킹
- 기초 HLSL 셰이더 컴파일 + 정점/인덱스 버퍼 바인딩 + `DrawIndexed`

### 📌 [Phase 2] 객체 코어 & 이동 — 변환 행렬 + 프레임워크 토대

- **결과:** 방향키 입력 시 사각형이 프레임 독립적으로 부드럽게 이동, ImGui에 X/Y 좌표 실시간 표시
- 상수 버퍼 도입 + `MatrixWorld` 계산
- **`UObject`** 구현 → **🆕 핸들/수명(PendingKill) 시스템 함께 설계**
- **🆕 멀티캐스트 델리게이트 템플릿** 함께 구현 (이후 전방위로 사용)
- **`AActor`** + 위치를 가질 **`USceneComponent2D`** 루트 컴포넌트
- 메인 루프에서 `Actor->Tick(DeltaTime)` 구동
- ImGui에 선택 액터의 Transform 데이터 연동

### 📌 [Phase 3] 텍스처 & 스프라이트 — 렌더 컴포넌트 분리 + 정렬

- **결과:** 투명 배경 PNG 캐릭터 출력, ImGui로 텍스처 리스트 모니터링 + 알파 조절
- DirectXTex 연동 + `ShaderResourceView(SRV)` 로드
- 알파 블렌딩(`BlendState`) 적용
- 그리기 로직을 `USpriteComponent`로 모듈화 분리
- **🆕 sort key 기반 드로우 정렬 도입** (레이어 + Y/Z)
- ImGui: 액터 목록(Scenery) 창 + 속성 편집 디테일 패널 기초
- **🆕 결정 필요:** 디테일 패널을 범용 리플렉션으로 갈지(매크로/템플릿 프로퍼티 등록) vs 컴포넌트별 ImGui 수작업으로 갈지 → **Phase 3 진입 전 확정**

### 📌 [Phase 4] 스프라이트 애니메이션 — UV 연산

- **결과:** 스프라이트 시트에서 프레임이 순차 변경되며 캐릭터가 살아 움직임, ImGui로 일시정지/프레임 스텝 디버깅
- 누적 시간 → 셰이더로 보낼 UV Offset(행/열 인덱스) 연산
- 언리얼 Flipbook을 본뜬 `UFlipbookComponent2D` (PlayRate, Loop, 현재 프레임)
- ImGui 제어 바로 실시간 에셋 뷰어

### 📌 [Phase 5] 입력 분리 & 카메라 — Pawn / Controller / Camera (탐험 토대)

- **결과:** 캐릭터 이동에 따라 카메라가 월드를 스크롤
- `APlayerController` 구현 + `APawn` 빙의(`Possess`) 구조
- `UCameraComponent` 구현 + View 행렬 GPU 송신
Pndc=Plocal×MWorld×MView×MProjection
    
    Pndc=Plocal×MWorld×MView×MProjectionP_{ndc} = P_{local} \times M_{World} \times M_{View} \times M_{Projection}
    
- ImGui 카메라 디버그 패널(오프셋 변경으로 스크롤 테스트)

### 📌 [Phase 6] 그리드 맵 + 충돌 + AI (탐험 게임플레이)

- **결과:** 펄린노이즈 맵에서 플레이어가 그리드 이동, 벽 충돌, 몬스터가 추적하다 조우 시 전투 전환
- 펄린노이즈 맵 생성 (시드 기반 → 세이브/로드·재현성)
- `UPrimitiveComponent2D` → `UBoxComponent2D`(AABB) 구현
- 충돌 시 **`OnComponentBeginOverlap` 델리게이트 브로드캐스트** (Phase 2의 델리게이트 재사용)
- `AIController` + A* 추적 (⚠️ **계층적 길찾기는 미리 하지 말 것** — 실제로 느려진 게 측정된 다음 도입)
- 탐험 ↔ 전투 상태 전환 FSM 매니저
- ImGui: 충돌 디버그 드로우 On/Off

### 📌 [Phase 7] World / Level / GameMode (게임 규칙 & 월드 관리)

- **결과:** 타이틀 → 인게임 전환, 시작 시 캐릭터/몬스터 자동 스폰, 클리어 조건 감시
- 씬 구조를 대체할 `UWorld` / `ULevel`
- 규칙·승패 조건 관리 상위 객체 `AGameModeBase`
- ImGui QA 치트 시스템(무적, 강제 드로우, 적 즉사, 강제 턴 넘김, 스테이지 이동)

---

## ⚔️ 8. [별도 트랙] 전투 시스템 — 데이터 주도 (Engine 아님, Game 상층) <변경 가능성 있음>

> Actor::Tick이 아니라 **커맨드 패턴 + 턴 FSM + 데이터 테이블**로 구동. 가장 먼저 종이에 설계해둘 것.
> 
- **카드 데이터 정의 (CSV):** 카드 스탯/효과/코스트를 C++에 박지 않고 CSV로 분리 → `DataTableLoader` 필요. (덱빌더 작업의 절반)
- **카드 효과 = 커맨드:** 각 효과를 커맨드 객체로 표현(데미지/방어/드로우/버프 등), 실행 큐로 처리
- **턴 상태 머신:** Draw → Play → Resolve → Enemy → End FSM
- **자료구조:** 덱(Stack/Queue), 손패(List), 버림더미(Stack)
- **선공 결정:** 몬스터 타입에 따라 (우선 플레이어 선공으로 시작)
- **입력:** 키보드 선택 → 이후 마우스 드래그앤드롭으로 확장

### 재사용성 등급 (어디에 둘지 판단 기준)

- **거의 모든 2D 게임에 재사용** → 마우스 피킹/히트테스트, 위젯(UI) 시스템, 드로우 정렬, DataTable 로더 → **Engine 또는 Gameplay 하단**
- **카드게임류에만 재사용** → 턴 FSM, 코스트/에너지, 덱/손패/버림 → **Gameplay 상층**
- **이 게임에만** → 특정 카드 효과, 특정 적 패턴 → **Game.exe**

---

## 🌄 9. [스트레치] 레이캐스팅 1인칭 뷰

- 펄린노이즈 격자 위 레이캐스팅(가로 픽셀마다 레이 → 벽 세로 스트립)
- **DDA 알고리즘**으로 격자 탐색 최적화
- 몬스터 = 3D 공간 속 2D **빌보드** (플레이어를 바라보고, 거리 기반 스케일링)
- ⚠️ SpriteComponent 씬 그래프에 욱여넣지 말 것 — **별개의 상위 렌더 경로**, 단 RHI 쿼드 드로우는 공유

## 🖱️ 10. [스트레치] 마우스 드래그앤드롭

- 위젯/히트테스트 기반 카드 드래그앤드롭

---

## 🔁 11. 프레임 라이프사이클 (Engine::Tick)

```cpp
[ 프레임 시작 ]
   │
1. [PreTick]   : UInputSubsystem이 키보드/마우스 하드웨어 상태 스냅샷 갱신
   │
2. [Tick]      : UWorld → ULevel → Actors 순회하며 각 Actor/Component Tick(DeltaTime)
   │             (Controller가 입력 해석 → Pawn 제어, AIController FSM 수행)
   │
3. [FixedTick] : (필요 시) 누적기(accumulator) 기반 고정 스텝으로 0~N회 구동
   │             ※ 본 게임은 고정스텝 물리 거의 불필요 → 충돌은 그리드/히트테스트 수준이면 단순화 가능
   │
4. [PostTick]  : PendingKill 마킹된 무효 액터 수거 및 메모리 해제 (핸들 무효화)
   │
5. [Render]    : Camera View/Proj 갱신 → sort key 정렬 → 드로우당 CB 갱신하며 GPU 드로우 콜
   │
[ 프레임 종료 / Present ]
```
