#include "pch.h"
#include "ShapeComponent.h"
#include "OBB.h"
#include "Collision.h"
#include "World.h"
#include "WorldPartitionManager.h"
#include "BVHierarchy.h"
#include "GameObject.h"
#include "Collision/CollisionManager.h"

// IMPLEMENT_CLASS is now auto-generated in .generated.cpp
UShapeComponent::UShapeComponent() : bShapeIsVisible(true), bShapeHiddenInGame(true)
{
    ShapeColor = FVector4(0.2f, 0.8f, 1.0f, 1.0f);
    bCanEverTick = true;
    bGenerateOverlapEvents = true;
    bDrawOnlyIfSelected = false;
}

UShapeComponent::~UShapeComponent()
{
    OverlapInfos.clear();
}

void UShapeComponent::BeginPlay()
{
    Super::BeginPlay();

    // World의 CollisionManager에 등록
    if (AActor* Owner = GetOwner())
    {
        if (UWorld* World = Owner->GetWorld())
        {
            if (UCollisionManager* Manager = World->GetCollisionManager())
            {
                Manager->RegisterComponent(this);
            }
        }
    }
}

void UShapeComponent::EndPlay(EEndPlayReason Reason)
{
    Super::EndPlay(Reason);

    // World의 CollisionManager에서 해제
    if (AActor* Owner = GetOwner())
    {
        if (UWorld* World = Owner->GetWorld())
        {
            if (UCollisionManager* Manager = World->GetCollisionManager())
            {
                Manager->UnregisterComponent(this);
            }
        }
    }
}

void UShapeComponent::OnRegister(UWorld* InWorld)
{
    Super::OnRegister(InWorld);

    GetWorldAABB();
    UpdateBounds();
}

void UShapeComponent::OnTransformUpdated()
{
    GetWorldAABB();
    UpdateBounds();

    // Keep BVH up-to-date for broad phase queries
    if (UWorld* World = GetWorld())
    {
        if (UWorldPartitionManager* Partition = World->GetPartitionManager())
        {
            Partition->MarkDirty(this);
        }
    }

    // CollisionManager에 Dirty 마킹
    OnTransformChanged();

    Super::OnTransformUpdated();
}

void UShapeComponent::OnTransformChanged()
{
    // Bounds 업데이트
    UpdateBounds();

    // World의 CollisionManager에 Dirty 마킹
    if (AActor* Owner = GetOwner())
    {
        if (UWorld* World = Owner->GetWorld())
        {
            if (UCollisionManager* Manager = World->GetCollisionManager())
            {
                Manager->MarkComponentDirty(this);
            }
        }
    }
}

void UShapeComponent::TickComponent(float DeltaSeconds)
{
    // 기본 ShapeComponent 클래스 자체는 overlap 이벤트를 생성하지 않음
    if (GetClass() == UShapeComponent::StaticClass())
    {
        bGenerateOverlapEvents = false;
    }

    if (!bGenerateOverlapEvents)
    {
        OverlapInfos.clear();
    }

    // NOTE: 충돌 처리는 이제 CollisionManager에서 수행됨
    // TickComponent에서 직접 충돌 처리를 하지 않음
}

// ────────────────────────────────────────────────────────────────────────────
// Bounds 관련 함수
// ────────────────────────────────────────────────────────────────────────────

void UShapeComponent::UpdateBounds()
{
    // 기본 구현: AABB 기반으로 Bounds 업데이트
    WorldAABB = GetWorldAABB();
    CachedBounds = FBoxSphereBounds(WorldAABB.GetCenter(), WorldAABB.GetHalfExtent());
}

FBoxSphereBounds UShapeComponent::GetScaledBounds() const
{
    return CachedBounds;
}

FAABB UShapeComponent::GetWorldAABB() const
{
    if (AActor* Owner = GetOwner())
    {
        FAABB OwnerBounds = Owner->GetBounds();
        const FVector HalfExtent = OwnerBounds.GetHalfExtent();
        WorldAABB = OwnerBounds;
    }
    return WorldAABB;
}

// ────────────────────────────────────────────────────────────────────────────
// Overlap 관련 함수 구현
// ────────────────────────────────────────────────────────────────────────────

bool UShapeComponent::IsOverlappingComponent(const UShapeComponent* Other) const
{
    if (!Other || !bGenerateOverlapEvents || !Other->bGenerateOverlapEvents)
    {
        return false;
    }

    // Bounds 기반 빠른 체크
    FBoxSphereBounds MyBounds = GetScaledBounds();
    FBoxSphereBounds OtherBounds = Other->GetScaledBounds();

    if (!MyBounds.Intersects(OtherBounds))
    {
        return false;
    }

    // Narrow phase: 실제 Shape 충돌 체크
    return Collision::CheckOverlap(const_cast<UShapeComponent*>(this), const_cast<UShapeComponent*>(Other));
}

void UShapeComponent::UpdateOverlaps(const TArray<UShapeComponent*>& OtherComponents)
{
    if (!bGenerateOverlapEvents)
    {
        return;
    }

    // 컴포넌트 소유자가 파괴 예정인지 먼저 확인
    AActor* Owner = GetOwner();
    if (!Owner || Owner->IsPendingKill())
    {
        return;
    }

    UWorld* World = Owner->GetWorld();
    if (!World)
    {
        return;
    }

    // 1단계: 현재 프레임에서 겹쳐있는 컴포넌트 확인
    TArray<UShapeComponent*> CurrentOverlaps;

    for (UShapeComponent* OtherComp : OtherComponents)
    {
        if (OtherComp == this)
        {
            continue; // 자기 자신은 제외
        }

        if (!OtherComp || !OtherComp->bGenerateOverlapEvents)
        {
            continue;
        }

        // 같은 Owner끼리는 충돌 무시
        if (OtherComp->GetOwner() == Owner)
        {
            continue;
        }

        if (IsOverlappingComponent(OtherComp))
        {
            CurrentOverlaps.push_back(OtherComp);
        }
    }

    // 2단계: 새로 시작된 Overlap 감지 (Begin)
    for (UShapeComponent* OtherComp : CurrentOverlaps)
    {
        FOverlapInfo* ExistingInfo = FindOverlapInfo(OtherComp);

        if (!ExistingInfo)
        {
            // 새로운 Overlap 시작
            AActor* OtherActor = OtherComp->GetOwner();
            FVector ContactPoint = (GetScaledBounds().Origin + OtherComp->GetScaledBounds().Origin) * 0.5f;
            float PenetrationDepth = 0.0f; // 추후 정밀 계산 추가 가능

            FOverlapInfo NewInfo(OtherComp, OtherActor, ContactPoint, PenetrationDepth, false);
            OverlapInfos.push_back(NewInfo);

            // 충돌 상태 업데이트
            bIsOverlapping = true;

            // 이전에 호출된 적이 있는 이벤트 인지 확인
            if (Owner && OtherActor && World->TryMarkOverlapPair(Owner, OtherActor))
            {
                // 양방향 호출 (Actor 델리게이트)
                Owner->OnComponentBeginOverlap.Broadcast(this, OtherComp);
                OtherActor->OnComponentBeginOverlap.Broadcast(OtherComp, this);

                // Hit호출
                Owner->OnComponentHit.Broadcast(this, OtherComp);
                if (bBlockComponent)
                {
                    OtherActor->OnComponentHit.Broadcast(OtherComp, this);
                }
            }

            // Component 델리게이트 호출
            OnComponentBeginOverlap.Broadcast(this, OtherActor, OtherComp, ContactPoint, PenetrationDepth);

            // Broadcast 후 this가 파괴되었는지 확인
            Owner = GetOwner();
            if (!Owner || Owner->IsPendingKill())
            {
                return;
            }
        }
    }

    // 3단계: 끝난 Overlap 감지 (End)
    TArray<UShapeComponent*> OverlapsToRemove;

    for (const FOverlapInfo& Info : OverlapInfos)
    {
        bool bStillOverlapping = false;

        for (UShapeComponent* CurrentComp : CurrentOverlaps)
        {
            if (Info.OtherComponent == CurrentComp)
            {
                bStillOverlapping = true;
                break;
            }
        }

        if (!bStillOverlapping)
        {
            // Overlap 종료
            OverlapsToRemove.push_back(Info.OtherComponent);

            // 이전에 호출된 적이 있는 이벤트 인지 확인
            if (Owner && Info.OtherActor && World->TryMarkOverlapPair(Owner, Info.OtherActor))
            {
                // 양방향 호출 (Actor 델리게이트)
                Owner->OnComponentEndOverlap.Broadcast(this, Info.OtherComponent);
                Info.OtherActor->OnComponentEndOverlap.Broadcast(Info.OtherComponent, this);
            }

            // Component 델리게이트 호출
            OnComponentEndOverlap.Broadcast(
                this,
                Info.OtherActor,
                Info.OtherComponent,
                Info.ContactPoint,
                Info.PenetrationDepth
            );

            // Broadcast 후 this가 파괴되었는지 확인
            Owner = GetOwner();
            if (!Owner || Owner->IsPendingKill())
            {
                return;
            }
        }
    }

    // 4단계: 종료된 Overlap 정보 제거
    for (UShapeComponent* CompToRemove : OverlapsToRemove)
    {
        RemoveOverlapInfo(CompToRemove);
    }

    // 5단계: 충돌 상태 업데이트
    bIsOverlapping = !OverlapInfos.empty();
}

FOverlapInfo* UShapeComponent::FindOverlapInfo(const UShapeComponent* OtherComponent)
{
    for (FOverlapInfo& Info : OverlapInfos)
    {
        if (Info.OtherComponent == OtherComponent)
        {
            return &Info;
        }
    }

    return nullptr;
}

bool UShapeComponent::RemoveOverlapInfo(const UShapeComponent* OtherComponent)
{
    for (auto It = OverlapInfos.begin(); It != OverlapInfos.end(); ++It)
    {
        if (It->OtherComponent == OtherComponent)
        {
            OverlapInfos.erase(It);
            return true;
        }
    }

    return false;
}

void UShapeComponent::DuplicateSubObjects()
{
    Super::DuplicateSubObjects();

    // Overlap 정보는 복사하지 않음 (런타임에 새로 생성됨)
    OverlapInfos.clear();
}
