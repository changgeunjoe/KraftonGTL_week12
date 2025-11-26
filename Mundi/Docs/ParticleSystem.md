# Mundi 파티클 시스템 가이드

## 목차
1. [개요](#1-개요)
2. [파일 구조](#2-파일-구조)
3. [핵심 클래스](#3-핵심-클래스)
4. [시스템 흐름](#4-시스템-흐름)
5. [파티클 모듈](#5-파티클-모듈)
6. [파티클 타입](#6-파티클-타입)
7. [메모리 구조](#7-메모리-구조)
8. [페이로드 시스템](#8-페이로드-시스템)
9. [ParticleHelper 역할](#9-particlehelper-역할)
10. [CPU → GPU 데이터 전달](#10-cpu--gpu-데이터-전달)
11. [렌더링 파이프라인](#11-렌더링-파이프라인)
12. [사용 예시](#12-사용-예시)

---

## 1. 개요

Mundi 파티클 시스템은 **모듈식 아키텍처**를 기반으로 설계되어, 다양한 파티클 효과를 유연하게 구현할 수 있습니다.

### 주요 특징
- **모듈식 설계**: 색상, 크기, 속도 등을 독립적인 모듈로 관리
- **LOD 지원**: Level of Detail을 통한 거리별 품질 조절
- **GPU 인스턴싱**: 대량의 파티클을 효율적으로 렌더링
- **JSON 직렬화**: `.particle` 파일로 저장/로드 가능

### 계층 구조

```
UParticleSystem (자산)
├── UParticleEmitter[] (이미터 템플릿)
│   └── UParticleLODLevel[] (LOD별 설정)
│       ├── UParticleModuleRequired (필수 설정)
│       ├── UParticleModuleSpawn (스폰 규칙)
│       └── UParticleModule[] (기타 모듈들)
│
UParticleSystemComponent (런타임)
├── FParticleEmitterInstance[] (이미터 인스턴스)
│   └── FBaseParticle[] (개별 파티클 데이터)
└── FDynamicEmitterData[] (렌더링 데이터)
```

---

## 2. 파일 구조

### 핵심 파일

| 경로 | 설명 |
|------|------|
| `Engine/Particle/ParticleSystem.h/cpp` | 파티클 시스템 자산 클래스 |
| `Engine/Particle/ParticleEmitter.h/cpp` | 이미터 템플릿 |
| `Engine/Particle/ParticleEmitterInstances.h/cpp` | 런타임 이미터 인스턴스 |
| `Engine/Components/ParticleSystemComponent.h/cpp` | 렌더링 컴포넌트 |

### 모듈 파일

| 경로 | 설명 |
|------|------|
| `Engine/Particle/ParticleModule.h/cpp` | 모듈 베이스 클래스 |
| `Engine/Particle/ParticleModuleRequired.h/cpp` | 필수 설정 모듈 |
| `Engine/Particle/ParticleModuleSpawn.h/cpp` | 스폰 속도 모듈 |
| `Engine/Particle/ParticleModuleLifetime.h/cpp` | 수명 모듈 |
| `Engine/Particle/ParticleModuleColor.h/cpp` | 색상 모듈 |
| `Engine/Particle/ParticleModuleSize.h/cpp` | 크기 모듈 |
| `Engine/Particle/ParticleModuleVelocity.h/cpp` | 속도 모듈 |

### 이미터 타입 파일

| 경로 | 설명 |
|------|------|
| `Engine/Particle/ParticleSpriteEmitter.h/cpp` | 스프라이트 이미터 |
| `Engine/Particle/ParticleModuleTypeDataMesh.h/cpp` | 메시 이미터 타입 |

---

## 3. 핵심 클래스

### 3.1 UParticleSystem

파티클 효과의 **자산(Asset)** 클래스입니다. 모든 이미터와 설정을 포함합니다.

```cpp
UCLASS()
class UParticleSystem : public UResourceBase
{
    TArray<UParticleEmitter*> Emitters;  // 이미터 배열
    FAABB FixedRelativeBoundingBox;       // 바운딩 박스
};
```

**주요 함수**:
- `AddEmitter<T>()` - 이미터 추가
- `LoadFromFile()` / `SaveToFile()` - JSON 파일 입출력
- `UpdateAllModuleLists()` - 모듈 리스트 갱신

### 3.2 UParticleSystemComponent

파티클을 **렌더링**하는 컴포넌트입니다. Actor에 부착하여 사용합니다.

```cpp
UCLASS(DisplayName = "파티클 컴포넌트")
class UParticleSystemComponent : public UPrimitiveComponent
{
    UParticleSystem* Template;                          // 파티클 자산
    TArray<FParticleEmitterInstance*> EmitterInstances; // 런타임 인스턴스
    TArray<FDynamicEmitterDataBase*> EmitterRenderData; // 렌더링 데이터
};
```

**주요 함수**:
- `SetTemplate()` - 파티클 자산 설정
- `Activate()` / `Deactivate()` - 활성화/비활성화
- `TickComponent()` - 매 프레임 업데이트
- `InitParticles()` - 파티클 초기화

### 3.3 UParticleEmitter

**이미터 템플릿** 클래스입니다. 파티클 생성 규칙을 정의합니다.

```cpp
UCLASS()
class UParticleEmitter : public UObject
{
    TArray<UParticleLODLevel*> LODLevels;  // LOD 레벨 배열
    FName EmitterName;                      // 이미터 이름
    int32 PeakActiveParticles;              // 최대 파티클 수
    int32 ParticleSize;                     // 파티클 데이터 크기 (바이트)
};
```

### 3.4 FParticleEmitterInstance

**런타임 시뮬레이션**을 담당하는 구조체입니다.

```cpp
struct FParticleEmitterInstance
{
    uint8* ParticleData;         // 파티클 데이터 배열
    uint16* ParticleIndices;     // 활성 파티클 인덱스
    int32 ActiveParticles;       // 현재 활성 파티클 수
    int32 MaxActiveParticles;    // 최대 할당 파티클 수
    float EmitterTime;           // 현재 이미터 시간
    float EmitterDuration;       // 이미터 지속 시간
};
```

### 3.5 FBaseParticle

개별 **파티클의 데이터**를 저장하는 구조체입니다 (144 바이트).

```cpp
struct FBaseParticle
{
    FVector OldLocation;      // 이전 위치
    FVector Location;         // 현재 위치
    FVector BaseVelocity;     // 기본 속도
    FVector Velocity;         // 현재 속도
    FVector BaseSize;         // 기본 크기
    FVector Size;             // 현재 크기
    float Rotation;           // 회전 (라디안)
    float RotationRate;       // 회전 속도
    FLinearColor Color;       // 현재 색상
    FLinearColor BaseColor;   // 기본 색상
    float RelativeTime;       // 상대 시간 (0~1)
    float OneOverMaxLifetime; // 수명의 역수
    int32 Flags;              // 상태 플래그
};
```

---

## 4. 시스템 흐름

### 4.1 초기화 흐름

```
SetTemplate() 호출
    ↓
InitializeSystem()
    ↓
InitParticles()
    ├── 각 이미터마다 CreateInstance() 호출
    ├── FParticleEmitterInstance 생성
    ├── ParticleData 메모리 할당
    └── ParticleIndices 초기화
```

### 4.2 업데이트 흐름 (매 프레임)

```
TickComponent(DeltaTime)
    ↓
for each EmitterInstance:
    │
    ├── 1. Tick_EmitterTimeSetup()
    │       └── EmitterTime 업데이트, 루프 처리
    │
    ├── 2. KillParticles()
    │       └── 수명 다한 파티클 제거
    │
    ├── 3. ResetParticleParameters()
    │       └── Velocity = BaseVelocity, Size = BaseSize
    │
    ├── 4. Tick_ModuleUpdate()
    │       └── 각 UpdateModule의 Update() 호출
    │
    ├── 5. Tick_SpawnParticles()
    │       ├── SpawnModule.GetSpawnAmount() → 생성 개수 계산
    │       └── SpawnParticles() 호출
    │           ├── PreSpawn() - 메모리 할당
    │           ├── 각 SpawnModule의 Spawn() 호출
    │           └── PostSpawn() - 초기화 완료
    │
    ├── 6. Tick_ModulePostUpdate()
    │
    ├── 7. UpdateBoundingBox()
    │       └── Location += Velocity * DeltaTime
    │
    └── 8. Tick_ModuleFinalUpdate()
```

### 4.3 렌더링 흐름

```
UpdateDynamicData()
    ↓
for each EmitterInstance:
    │
    ├── GetDynamicData() 호출
    │   ├── FDynamicSpriteEmitterData 생성
    │   ├── GetReplayData() → FParticleSpriteVertex[] 생성
    │   └── GPU 버퍼에 업로드
    │
    ↓
CollectMeshBatches()
    │
    └── GetDynamicMeshElementsEmitter()
        ├── 스프라이트: DrawIndexedInstanced (인스턴싱)
        └── 메시: 각 파티클마다 메시 렌더링
```

---

## 5. 파티클 모듈

### 5.1 모듈 베이스

```cpp
UCLASS()
class UParticleModule : public UObject
{
    uint8 bSpawnModule : 1;       // Spawn 시 실행
    uint8 bUpdateModule : 1;      // Update 시 실행
    uint8 bFinalUpdateModule : 1; // FinalUpdate 시 실행
    uint8 bEnabled : 1;           // 활성화 여부
};
```

### 5.2 모듈 타입

| 타입 | 설명 |
|------|------|
| `EPMT_Required` | 필수 모듈 (머티리얼, 정렬 등) |
| `EPMT_Spawn` | 스폰 속도 모듈 |
| `EPMT_TypeData` | 이미터 타입 (Sprite, Mesh) |
| `EPMT_General` | 일반 모듈 (색상, 크기, 속도) |

### 5.3 필수 모듈 (UParticleModuleRequired)

모든 이미터에 반드시 필요한 기본 설정을 담당합니다.

```cpp
class UParticleModuleRequired : public UParticleModule
{
    UMaterial* Material;                    // 파티클 머티리얼
    EParticleScreenAlignment ScreenAlignment; // 화면 정렬
    bool bUseLocalSpace;                    // 로컬/월드 공간
    float EmitterDuration;                  // 이미터 지속 시간
    int32 EmitterLoops;                     // 반복 횟수 (0=무한)
    FVector2D SubImages;                    // SubUV 그리드 (X, Y)
};
```

**화면 정렬 옵션**:
- `PSA_FacingCameraPosition` - 카메라를 향함
- `PSA_Square` - 정사각형
- `PSA_Rectangle` - 직사각형
- `PSA_Velocity` - 속도 방향 정렬

### 5.4 스폰 모듈 (UParticleModuleSpawn)

파티클 **생성 속도**를 제어합니다.

```cpp
class UParticleModuleSpawn : public UParticleModuleSpawnBase
{
    FRawDistributionFloat Rate;     // 초당 생성 개수
    FRawDistributionFloat RateScale; // 스케일 배수
    int32 BurstCount;               // 버스트 생성 개수
};
```

**계산 공식**:
```
SpawnCount = (Rate * DeltaTime) + Leftover
```

### 5.5 수명 모듈 (UParticleModuleLifetime)

파티클의 **생존 시간**을 설정합니다.

```cpp
class UParticleModuleLifetime : public UParticleModuleLifetimeBase
{
    FRawDistributionFloat Lifetime; // 수명 분포
};
```

### 5.6 색상 모듈 (UParticleModuleColor)

파티클의 **색상**을 설정합니다.

```cpp
class UParticleModuleColor : public UParticleModule
{
    FRawDistributionVector StartColor; // 시작 색상 (RGB)
    FRawDistributionFloat StartAlpha;  // 시작 알파
};
```

### 5.7 크기 모듈 (UParticleModuleSize)

파티클의 **크기**를 설정합니다.

```cpp
class UParticleModuleSize : public UParticleModule
{
    FRawDistributionVector StartSize; // 시작 크기 (X, Y, Z)
};
```

### 5.8 속도 모듈 (UParticleModuleVelocity)

파티클의 **초기 속도**를 설정합니다.

```cpp
class UParticleModuleVelocity : public UParticleModule
{
    FRawDistributionVector StartVelocity;       // 시작 속도
    FRawDistributionFloat StartVelocityRadial;  // 방사형 속도
};
```

### 5.9 모듈 호출 구조

모듈의 `Spawn()`, `Update()`, `FinalUpdate()` 함수가 언제 호출되는지 보여줍니다.

```
FParticleEmitterInstance::Tick()
    ├→ Tick_ModuleUpdate()           // bUpdateModule=true인 모듈
    │   └→ Module->Update()
    │
    ├→ Tick_SpawnParticles()
    │   └→ SpawnParticles()
    │       └→ for(SpawnModules)
    │           └→ Module->Spawn()   // bSpawnModule=true인 모듈
    │
    └→ Tick_ModuleFinalUpdate()      // bFinalUpdateModule=true인 모듈
        └→ Module->FinalUpdate()
```

### 5.10 모듈별 영향 범위

| 모듈 | 호출 시점 | 영향 대상 |
|------|----------|----------|
| **Velocity** | Spawn | Particle.Velocity, BaseVelocity |
| **Color** | Spawn | Particle.Color, BaseColor |
| **Size** | Spawn | Particle.Size, BaseSize |
| **Lifetime** | Spawn | OneOverMaxLifetime, RelativeTime |
| **SubUV** | Spawn/Update | SubImageIndex |

---

## 6. 파티클 타입

### 6.1 스프라이트 파티클 (UParticleSpriteEmitter)

2D 이미지 기반 파티클입니다. 카메라를 향하는 빌보드 형태로 렌더링됩니다.

```cpp
class UParticleSpriteEmitter : public UParticleEmitter
{
    // 스프라이트 이미터 인스턴스 생성
    FParticleEmitterInstance* CreateInstance(UParticleSystemComponent* InComponent) override;
};
```

**GPU 데이터 구조**:
```cpp
struct FParticleSpriteVertex
{
    FVector Position;       // 위치
    FVector2D Size;         // 크기
    float Rotation;         // 회전
    FLinearColor Color;     // 색상
    float RelativeTime;     // 상대 시간
};
```

### 6.2 메시 파티클 (UParticleModuleTypeDataMesh)

3D 메시를 파티클로 사용합니다.

```cpp
class UParticleModuleTypeDataMesh : public UParticleModuleTypeDataBase
{
    UStaticMesh* Mesh;                   // 렌더링할 메시
    EMeshScreenAlignment MeshAlignment;  // 메시 정렬 방식
};
```

**메시 정렬 옵션**:
- `PSMA_MeshFaceCameraWithRoll` - 카메라 정렬 + Roll
- `PSMA_MeshFaceCameraWithSpin` - 카메라 정렬 + Spin
- `PSMA_MeshFaceCameraWithLockedAxis` - 잠긴 축

---

## 7. 메모리 구조

### 7.1 파티클 데이터 레이아웃

각 파티클의 메모리 구조:

```
┌─────────────────────────────────────┐
│ FBaseParticle (144 바이트, 고정)     │
├─────────────────────────────────────┤
│ 모듈 추가 데이터 (동적)              │
│ ├── ColorModule: 16 바이트          │
│ ├── SizeModule: 12 바이트           │
│ └── VelocityModule: 12 바이트       │
├─────────────────────────────────────┤
│ 16바이트 정렬 패딩                   │
└─────────────────────────────────────┘
```

**ParticleStride 계산**:
```cpp
ParticleStride = ((ParticleSize + 15) & ~15);  // 16바이트 정렬
```

### 7.2 파티클 풀 시스템

```
ParticleData 배열:
┌──────────────┐ ← Index 0
│ Particle[0]  │
├──────────────┤
│ Particle[1]  │
├──────────────┤
│ Particle[2]  │
└──────────────┘

ParticleIndices (활성 파티클):
[2, 0, 1] → 활성 순서: Particle[2], Particle[0], Particle[1]
```

---

## 8. 페이로드 시스템

### 8.1 페이로드란?

페이로드(Payload)는 **기본 파티클 데이터(`FBaseParticle`) 외에 모듈이 필요로 하는 추가 데이터**입니다.

### 8.2 파티클 메모리 레이아웃

```
┌─────────────────────────────────────────────────────────────┐
│                    파티클 메모리 구조                         │
├─────────────────────────────────────────────────────────────┤
│  FBaseParticle (기본 데이터) - 144 bytes                     │
│  ├── OldLocation, Location (위치)                           │
│  ├── Velocity, BaseVelocity (속도)                          │
│  ├── Size, BaseSize (크기)                                  │
│  ├── Color, BaseColor (색상)                                │
│  ├── Rotation, RotationRate (회전)                          │
│  └── RelativeTime, OneOverMaxLifetime, SubImageIndex        │
├─────────────────────────────────────────────────────────────┤
│  PayloadOffset ← 여기서부터 페이로드 시작                     │
├─────────────────────────────────────────────────────────────┤
│  모듈 페이로드 데이터 (모듈마다 다름)                          │
│  예: FMeshRotationPayloadData - 메시 파티클용 3D 회전 데이터   │
│      ├── InitialOrientation (초기 방향)                      │
│      ├── Rotation (현재 회전)                                │
│      ├── RotationRate (회전 속도)                            │
│      └── CurContinuousRotation (누적 회전)                   │
├─────────────────────────────────────────────────────────────┤
│  다른 모듈의 추가 페이로드...                                  │
└─────────────────────────────────────────────────────────────┘
```

### 8.3 페이로드가 필요한 이유

1. **기본 파티클 구조는 고정됨**: `FBaseParticle`은 모든 파티클이 공유하는 기본 속성만 가짐
2. **모듈마다 추가 데이터 필요**:
   - 메시 파티클: 3D 회전 데이터 (`FMeshRotationPayloadData`)
   - 리본/트레일: 연결 정보, 히스토리 데이터
   - 커스텀 모듈: 사용자 정의 데이터

### 8.4 모듈 추가 시 버퍼 관리

모듈이 추가될 때 `ParticleEmitter.cpp`의 `CacheEmitterModuleInfo()`에서 버퍼 크기를 계산합니다.

```cpp
void UParticleEmitter::CacheEmitterModuleInfo()
{
    ParticleSize = sizeof(FBaseParticle);  // 기본 크기로 시작
    ReqInstanceBytes = 0;

    for (각 모듈)
    {
        // 1. 파티클당 페이로드 계산
        int32 ReqBytes = ParticleModule->RequiredBytes(HighTypeData);
        if (ReqBytes)
        {
            ModuleOffsetMap.Add(ParticleModule, ParticleSize);  // 오프셋 저장
            ParticleSize += ReqBytes;                           // 크기 누적
        }

        // 2. 인스턴스당 데이터 계산
        int32 TempInstanceBytes = ParticleModule->RequiredBytesPerInstance();
        if (TempInstanceBytes > 0)
        {
            ModuleInstanceOffsetMap.Add(ParticleModule, ReqInstanceBytes);
            ReqInstanceBytes += TempInstanceBytes;
        }
    }
}
```

### 8.5 페이로드 접근 예시

```cpp
// 메시 파티클에서 회전 페이로드 접근
FMeshRotationPayloadData* PayloadData =
    (FMeshRotationPayloadData*)((uint8*)&Particle + MeshRotationOffset);

PayloadData->Rotation = PayloadData->InitialOrientation +
                        PayloadData->CurContinuousRotation;
```

### 8.6 요약 테이블

| 구분 | 설명 |
|------|------|
| **FBaseParticle** | 모든 파티클이 가지는 기본 데이터 (위치, 속도, 크기, 색상 등) |
| **Payload** | 특정 모듈/타입이 필요로 하는 **추가 데이터** |
| **PayloadOffset** | 기본 데이터 끝나고 페이로드가 시작하는 위치 |
| **RequiredBytes()** | 모듈이 필요한 페이로드 크기 반환 |
| **ModuleOffsetMap** | 모듈 → 페이로드 오프셋 매핑 |

---

## 9. ParticleHelper 역할

### 9.1 개요

`ParticleHelper`는 **파티클 시스템과 렌더러 사이의 다리** 역할을 합니다. CPU에서 계산된 파티클 데이터를 GPU가 이해할 수 있는 형태로 변환합니다.

### 9.2 헬퍼 매크로

모듈에서 파티클 데이터에 접근할 때 사용하는 매크로들입니다.

```cpp
// 파티클 데이터 접근
DECLARE_PARTICLE(Name, Address)  // 주소에서 FBaseParticle 참조

// 스폰 시 파티클 접근 준비
SPAWN_INIT

// 모든 활성 파티클 순회 (Update에서 사용)
BEGIN_UPDATE_LOOP
    // Particle 변수로 각 파티클 접근
END_UPDATE_LOOP
```

### 9.3 GPU 정점 구조체

| 구조체 | 용도 | 주요 데이터 |
|--------|------|------------|
| **FParticleSpriteVertex** | 스프라이트 파티클 | Position, Size, Rotation, Color, UV |
| **FMeshParticleInstanceVertex** | 메시 파티클 | Transform(행렬), Color, SubUV |

### 9.4 동적 이미터 데이터

`FDynamicSpriteEmitterData`와 `FDynamicMeshEmitterData`가 실제 렌더링을 담당합니다.

**주요 기능**:
- CPU 파티클 데이터 → GPU 구조화 버퍼로 복사
- 카메라 거리 기준 정렬 (Back-to-Front, 투명도 처리용)
- 인스턴싱으로 렌더링 (1 Draw Call)

### 9.5 GPU 버퍼 동적 크기 조절

파티클 수가 증가하면 버퍼 크기를 **2배씩** 늘립니다.

```cpp
// ParticleHelper.cpp:138-149
if (ParticleCount > ParticleStructuredBufferSize)
{
    uint32 NewSize = ParticleStructuredBufferSize;
    while (NewSize < ParticleCount)
    {
        NewSize = FMath::Max(NewSize * 2, InitMaxParticles);  // 2배씩 증가
    }
    ParticleStructuredBufferSize = NewSize;
    CreateParticleStructuredBuffer(sizeof(FParticleSpriteVertex), NewSize);
}
```

**크기 변화 예시**:
```
초기: 128 → 파티클 200개 필요 → 256
     256 → 파티클 300개 필요 → 512
     ...
```

---

## 10. CPU → GPU 데이터 전달

### 10.1 핵심 개념

**페이로드 자체는 GPU로 넘기지 않습니다.** CPU에서 필요한 계산을 한 뒤 **결과값만** GPU 버퍼에 담아서 넘깁니다.

### 10.2 데이터 흐름

```
┌─────────────────────────────────────────────────────────────────┐
│  CPU 파티클 메모리                                               │
│  ┌────────────────────────────────────────────┐                 │
│  │ FBaseParticle (144 bytes)                  │                 │
│  │ ├── Location, Velocity, Size, Color...    │ ← GPU로 전달     │
│  ├────────────────────────────────────────────┤                 │
│  │ Module Payload (페이로드)                   │                 │
│  │ ├── FMeshRotationPayloadData              │ ← CPU에서 처리   │
│  │ └── 기타 모듈 데이터                        │   후 결과만 전달 │
│  └────────────────────────────────────────────┘                 │
└─────────────────────────────────────────────────────────────────┘
                          ↓
              ParticleHelper에서 변환
                          ↓
┌─────────────────────────────────────────────────────────────────┐
│  GPU 구조화 버퍼                                                 │
│  ├── Position      ← BaseParticle->Location                    │
│  ├── Size          ← BaseParticle->Size                         │
│  ├── Color         ← BaseParticle->Color                        │
│  ├── Transform     ← 페이로드에서 계산한 결과                     │
│  └── UV            ← SubImageIndex에서 계산                      │
└─────────────────────────────────────────────────────────────────┘
```

### 10.3 스프라이트 파티클 전달

```cpp
// ParticleHelper.cpp:172-189
for (int32 i = 0; i < ParticleCount; ++i)
{
    FBaseParticle* BaseParticle = ...;

    GpuParticles[i].Position = BaseParticle->Location;
    GpuParticles[i].Size = FVector2D(BaseParticle->Size.X, BaseParticle->Size.Y);
    GpuParticles[i].Color = BaseParticle->Color;

    // SubUV 계산
    GpuParticles[i].UVScale = UVScale;
    GpuParticles[i].UVOffset = ...;
}
```

### 10.4 메시 파티클 전달

```cpp
// ParticleHelper.cpp:303-312
for (int32 i = 0; i < ParticleCount; ++i)
{
    FBaseParticle* BaseParticle = ...;

    // 페이로드에서 회전 데이터 읽기
    FMeshRotationPayloadData* RotationPayload =
        (FMeshRotationPayloadData*)(BasePtr + SrcIndex * Stride + Src->MeshRotationOffset);

    // CPU에서 Transform 행렬 계산 후 GPU로 전달
    FTransform Transform(
        BaseParticle->Location,                            // 위치
        FQuat::MakeFromEulerZYX(RotationPayload->Rotation), // 페이로드의 회전
        BaseParticle->Size                                 // 크기
    );

    GpuParticles[i].Transform = Transform.ToMatrix();  // 결과만 GPU로
    GpuParticles[i].Color = BaseParticle->Color;
}
```

### 10.5 셰이더에서 데이터 접근

```hlsl
// ParticleSprite.hlsl
StructuredBuffer<FParticleSpriteVertex> ParticleBuffer : register(t10);

VS_OUTPUT VS(VS_INPUT Input, uint InstanceID : SV_InstanceID)
{
    FParticleSpriteVertex Particle = ParticleBuffer[InstanceID];

    // 각 인스턴스(파티클)마다 다른 데이터 사용
    float3 WorldPos = Particle.Position + Input.LocalPos * Particle.Size;
    ...
}
```

### 10.6 바인딩 흐름

```
CPU (ParticleHelper.cpp)
    ↓
memcpy(Mapped.pData, GpuParticles.GetData(), ...)
    ↓
ParticleStructuredBuffer (ID3D11Buffer)
    ↓
ParticleStructuredBufferSRV (ID3D11ShaderResourceView)
    ↓
BatchElement.ParticleDataSRV = ParticleStructuredBufferSRV
    ↓
렌더러에서 t10 슬롯에 바인딩
    ↓
GPU (셰이더에서 ParticleBuffer[InstanceID]로 접근)
```

### 10.7 데이터 전달 요약

| 데이터 | 처리 위치 | GPU 전달 방식 |
|--------|----------|--------------|
| Location, Size, Color | FBaseParticle | 직접 전달 |
| Rotation (페이로드) | CPU에서 계산 | Transform 행렬로 변환 후 전달 |
| SubImageIndex | FBaseParticle | UV 좌표로 변환 후 전달 |

---

## 11. 렌더링 파이프라인

### 11.1 스프라이트 렌더링

스프라이트 파티클은 **GPU 인스턴싱**을 사용합니다.

```
1. Quad 메시 (정적)
   ├── VertexBuffer: 4개 정점
   └── IndexBuffer: 6개 인덱스 (2 삼각형)

2. 인스턴스 데이터 (동적)
   └── ParticleStructuredBuffer: FParticleSpriteVertex[]

3. Draw Call
   └── DrawIndexedInstanced(6 indices, ActiveParticleCount instances)
```

### 11.2 메시 렌더링

메시 파티클은 **개별 메시 인스턴스**로 렌더링됩니다.

```cpp
struct FMeshParticleInstanceVertex
{
    FLinearColor Color;       // 색상
    FMatrix Transform;        // 변환 행렬
    FVector4 Velocity;        // 속도
    float RelativeTime;       // 상대 시간
};
```

---

## 12. 사용 예시

### 12.1 코드에서 파티클 생성

```cpp
// 1. 파티클 시스템 자산 생성
UParticleSystem* ParticleSystem = NewObject<UParticleSystem>();

// 2. 스프라이트 이미터 추가
UParticleSpriteEmitter* Emitter = ParticleSystem->AddEmitter<UParticleSpriteEmitter>();

// 3. LOD 레벨에서 모듈 설정
UParticleLODLevel* LOD = Emitter->LODLevels[0];

// 스폰 모듈 설정
LOD->SpawnModule->Rate.GetValue() = 10.0f;  // 초당 10개

// 수명 모듈 추가
UParticleModuleLifetime* Lifetime = LOD->AddModule<UParticleModuleLifetime>();
Lifetime->Lifetime.GetValue() = 2.0f;  // 2초

// 색상 모듈 추가
UParticleModuleColor* Color = LOD->AddModule<UParticleModuleColor>();
Color->StartColor.GetValue() = FVector(1, 0, 0);  // 빨간색
Color->StartAlpha.GetValue() = 1.0f;

// 4. 컴포넌트 생성 및 실행
UParticleSystemComponent* PSC = NewObject<UParticleSystemComponent>();
PSC->SetTemplate(ParticleSystem);
PSC->Activate(true);
```

### 12.2 JSON 파일에서 로드

```cpp
// 파티클 파일 로드
UParticleSystem* ParticleSystem = NewObject<UParticleSystem>();
ParticleSystem->LoadFromFile(L"Data/Particle/Test.particle");

// 컴포넌트에 적용
AParticleSystemActor* Actor = World->SpawnActor<AParticleSystemActor>();
Actor->ParticleSystemComponent->SetTemplate(ParticleSystem);
Actor->ParticleSystemComponent->Activate(true);
```

### 12.3 파티클 파일 구조 (.particle)

```json
{
    "Emitters": [
        {
            "ClassName": "UParticleSpriteEmitter",
            "EmitterName": "Main",
            "LODLevels": [
                {
                    "Level": 0,
                    "RequiredModule": {
                        "MaterialPath": "Data/Material/Particle.mat",
                        "ScreenAlignment": "PSA_FacingCameraPosition",
                        "EmitterDuration": 5.0,
                        "EmitterLoops": 0
                    },
                    "SpawnModule": {
                        "Rate": 20.0,
                        "RateScale": 1.0
                    },
                    "Modules": [
                        {
                            "ClassName": "UParticleModuleLifetime",
                            "Lifetime": 2.0
                        },
                        {
                            "ClassName": "UParticleModuleColor",
                            "StartColor": [1.0, 0.5, 0.0],
                            "StartAlpha": 1.0
                        }
                    ]
                }
            ]
        }
    ]
}
```

---

## 부록: 주요 함수 레퍼런스

### UParticleSystem

| 함수 | 설명 |
|------|------|
| `AddEmitter<T>()` | 타입별 이미터 추가 |
| `LoadFromFile(path)` | 파일에서 로드 |
| `SaveToFile(path)` | 파일로 저장 |
| `UpdateAllModuleLists()` | 모든 모듈 리스트 갱신 |
| `CalculateMaxActiveParticleCounts()` | 최대 파티클 수 계산 |

### UParticleSystemComponent

| 함수 | 설명 |
|------|------|
| `SetTemplate(system)` | 파티클 자산 설정 |
| `Activate(reset)` | 파티클 활성화 |
| `Deactivate()` | 파티클 비활성화 |
| `InitParticles()` | 파티클 초기화 |
| `ResetParticles()` | 파티클 제거 |
| `GetTotalActiveParticles()` | 활성 파티클 수 |

### FParticleEmitterInstance

| 함수 | 설명 |
|------|------|
| `Tick(deltaTime)` | 프레임 업데이트 |
| `SpawnParticles(count, time)` | 파티클 생성 |
| `KillParticles()` | 수명 다한 파티클 제거 |
| `GetDynamicData()` | 렌더링 데이터 생성 |
| `Resize(newMax)` | 메모리 재할당 |

### UParticleModule

| 함수 | 설명 |
|------|------|
| `Spawn(context)` | 파티클 생성 시 호출 |
| `Update(context)` | 파티클 업데이트 시 호출 |
| `RequiredBytes()` | 파티클당 추가 데이터 크기 |
| `SetToSensibleDefaults()` | 기본값 설정 |
