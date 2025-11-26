#include "pch.h"
#include "ParticleSystemComponent.h"

#include "BoxComponent.h"
#include "Collision.h"
#include "OBB.h"
#include "ParticleModuleLifetime.h"
#include "ParticleModuleVelocity.h"
#include "Source/Runtime/Engine/Particle/ParticleEmitter.h"
#include "Source/Runtime/Engine/Particle/ParticleEmitterInstances.h"
#include "Source/Runtime/Engine/Particle/ParticleHelper.h"
#include "Source/Runtime/Engine/Particle/ParticleLODLevel.h"
#include "Source/Runtime/Engine/Particle/ParticleSystem.h"
#include "Source/Runtime/Engine/Particle/ParticleSpriteEmitter.h"
#include "Source/Runtime/Engine/Particle/ParticleModuleTypeDataMesh.h"
#include "SceneView.h"
#include "BillboardComponent.h"

class UParticleModuleVelocity;

UParticleSystemComponent::UParticleSystemComponent()
	: Template(nullptr)
	, bSuppressSpawning(false)
	, bWasDeactivated(false)
	, bWasCompleted(true)
	, LODLevel(0)
	, TotalActiveParticles(0)
{
	bCanEverTick = true;	// 에디터에서 tick 돌리기 위한
}

UParticleSystemComponent::~UParticleSystemComponent()
{
	if (Template)
	{
		Template->OnParticleChanged.Remove(TemplateChangedHandle);
	}

	ResetParticles(true);
	ClearDynamicData();

	// Template은 ResourceManager가 관리하므로 여기서 삭제하지 않음
	Template = nullptr;
}

void UParticleSystemComponent::InitParticles()
{
	if (Template == nullptr)
	{
		return;
	}

	if (EmitterInstances.Num() > 0)
	{
		ResetParticles(true);
	}

	const int32 NumEmitters = Template->Emitters.Num();

	for (int32 Idx = 0; Idx < NumEmitters; Idx++)
	{
		UParticleEmitter* Emitter = Template->Emitters[Idx];
		if (Emitter)
		{
			FParticleEmitterInstance* Instance = Emitter->CreateInstance(this);
			if (Instance)
			{
				EmitterInstances.Add(Instance);

				Instance->InitParameters(Emitter);
				Instance->Init();
				Instance->CurrentLODLevelIndex = LODLevel;

				if (Emitter->LODLevels.Num() > 0)
				{
					Instance->CurrentLODLevel = Emitter->LODLevels[0];
				}
			}
		}

		bWasCompleted = false;
		bWasDeactivated = false;
		bSuppressSpawning = false;
	}
}

void UParticleSystemComponent::InitializeSystem()
{
	ResetParticles(true);

	if (Template == nullptr)
	{
		return;
	}

	ClearDynamicData();

	Template->UpdateAllModuleLists();

	InitParticles();
}

void UParticleSystemComponent::ResetParticles(bool bEmptyInstances)
{
	for (int32 i = 0; i < EmitterInstances.Num(); i++)
	{
		if (EmitterInstances[i])
		{
			delete EmitterInstances[i];
			EmitterInstances[i] = nullptr;
		}
	}

	if (bEmptyInstances)
	{
		EmitterInstances.Empty();
	}

	TotalActiveParticles = 0;
	bWasCompleted = true;
}

void UParticleSystemComponent::TickComponent(float DeltaTime)
{
	Super::TickComponent(DeltaTime);

	if (bWasCompleted && bWasDeactivated)
	{
		return;
	}

	if (Template && EmitterInstances.Num() == 0 && !bWasDeactivated)
	{
		InitParticles();
	}

	TotalActiveParticles = 0;

	for (int32 EmitterIndex = 0; EmitterIndex < EmitterInstances.Num(); EmitterIndex++)
	{
		FParticleEmitterInstance* Instance = EmitterInstances[EmitterIndex];

		if (Instance && Instance->SpriteTemplate)
		{
			assert(Instance->SpriteTemplate->LODLevels.Num() > 0);

			UParticleLODLevel* SpriteLODLevel = Instance->SpriteTemplate->GetCurrentLODLevel(Instance);
			if (SpriteLODLevel && SpriteLODLevel->bEnabled)
			{
				Instance->Tick(DeltaTime, bSuppressSpawning);

				TotalActiveParticles += Instance->ActiveParticles;
			}
		}
	}

	if (bWasDeactivated && TotalActiveParticles == 0)
	{
		bWasCompleted = true;
	}

	UpdateDynamicData();
}

void UParticleSystemComponent::CollectMeshBatches(TArray<FMeshBatchElement>& OutMeshBatchElements, const FSceneView* View)
{
	for (FDynamicEmitterDataBase* EmitterData : EmitterRenderData)
	{
		if (!EmitterData)
		{
			continue;
		}

		// 렌더링 배치를 수집 (Collect)
		EmitterData->GetDynamicMeshElementsEmitter(OutMeshBatchElements, View);
	}
}

void UParticleSystemComponent::DuplicateSubObjects()
{
	Super::DuplicateSubObjects();

	EmitterInstances.Empty();
	EmitterRenderData.Empty();
	SpriteComponent = nullptr;
	InitializeSystem();
}

void UParticleSystemComponent::Serialize(const bool bInIsLoading, JSON& InOutHandle)
{
	if (bInIsLoading)
	{
		SetTemplate(nullptr);
	}

	Super::Serialize(bInIsLoading, InOutHandle);

	if (bInIsLoading)
	{
		if (Template)
		{
			TemplateChangedHandle = Template->OnParticleChanged.AddDynamic(this, &UParticleSystemComponent::InitializeSystem);
		}

		InitializeSystem();

		if (Template)
		{
			Activate(true);
		}
	}
}

void UParticleSystemComponent::OnRegister(UWorld* InWorld)
{
	Super::OnRegister(InWorld);
	if (!SpriteComponent && !InWorld->bPie)
	{
		CREATE_EDITOR_COMPONENT(SpriteComponent, UBillboardComponent);
		SpriteComponent->SetTexture(GDataDir + "/UI/Icons/Particle.png");
	}
}

void UParticleSystemComponent::ClearDynamicData()
{
	EmitterRenderData.Empty();
}

void UParticleSystemComponent::UpdateDynamicData()
{
	ClearDynamicData();

	for (FParticleEmitterInstance* Instance : EmitterInstances)
	{
		if (Instance)
		{
			FDynamicEmitterDataBase* NewData = Instance->GetDynamicData(false);
			if (NewData)
			{
				EmitterRenderData.Add(NewData);
			}
		}
	}
}

void UParticleSystemComponent::Activate(bool bReset)
{
	if (Template == nullptr)
	{
		return;
	}

	if (bReset)
	{
		ResetParticles(bReset);
	}

	bWasDeactivated = false;
	bSuppressSpawning = false;
	bWasCompleted = false;

	if (EmitterInstances.Num() == 0)
	{
		InitParticles();
	}
}

void UParticleSystemComponent::Deactivate()
{
	bSuppressSpawning = true;
	bWasDeactivated = true;
}

bool UParticleSystemComponent::ParticleLineCheck(FHitResult& Hit, AActor* SourceActor, const FVector& End, const FVector& Start, const FVector& Extent)
{
	Hit.bBlockingHit = false;
	Hit.Time = 1.0f;

	FVector Direction = End - Start;
	float TraceLength = Direction.Size();
	if (TraceLength <= KINDA_SMALL_NUMBER)
	{
		return false;
	}
	FVector DirNormal = Direction / TraceLength;

	FRay Ray(Start, DirNormal);

	float MinDistance = TraceLength;
	bool bFoundHit = false;

	const TArray<AActor*>& Actors = GWorld->GetActors();
	for (AActor* Actor : Actors)
	{
		// 자기 자신(파티클을 내뿜는 액터)과의 충돌은 무시
		if (Actor == SourceActor)
		{
			continue;
		}

		for (USceneComponent* Component : Actor->GetSceneComponents())
		{
            UShapeComponent* ShapeComp = Cast<UShapeComponent>(Component);

            if (ShapeComp && ShapeComp->bBlockComponent)
            {
                FShape Shape;
                ShapeComp->GetShape(Shape);

                float Dist = FLT_MAX;
                FVector Normal;
                bool bHitShape = false;

                switch (Shape.Kind)
                {
                case EShapeKind::Box:
                    {
                        FOBB Obb;
                        Collision::BuildOBB(Shape, ShapeComp->GetWorldTransform(), Obb);

                        if (!Extent.IsZero())
                        {
                            float ParticleRadius = FMath::Max(Extent.X, FMath::Max(Extent.Y, Extent.Z));
                            Obb.HalfExtent[0] += ParticleRadius;
                            Obb.HalfExtent[1] += ParticleRadius;
                            Obb.HalfExtent[2] += ParticleRadius;
                        }

                        bHitShape = Collision::IntersectRayOBB(Ray, Obb, Dist, Normal);
                    }
                    break;

                case EShapeKind::Sphere:
                    // TODO: 나중에 IntersectRaySphere 구현 후 추가
                    break;

                case EShapeKind::Capsule:
                    // TODO: 나중에 IntersectRayCapsule 구현 후 추가
                    break;
                }

                if (bHitShape)
                {
                    if (Dist >= 0.0f && Dist < MinDistance)
                    {
                        MinDistance = Dist;
                        bFoundHit = true;

                        Hit.bBlockingHit = true;
                        Hit.Distance = Dist;
                        Hit.Time = Dist / TraceLength;
                        Hit.Location = Start + DirNormal * Dist;
                        Hit.Normal = Normal;
                        Hit.ImpactNormal = Normal;
                    }
                }
            }
		}
	}

	return bFoundHit;
}

void UParticleSystemComponent::SetTemplate(UParticleSystem* NewTemplate, bool bAutoActivate)
{
	// 기존 템플릿과 동일하면 아무것도 하지 않음
	if (Template == NewTemplate)
	{
		return;
	}

	if (Template)
	{
		Template->OnParticleChanged.Remove(TemplateChangedHandle);
	}

	// 새로운 템플릿 설정
	Template = NewTemplate;

	if (Template)
	{
		TemplateChangedHandle = Template->OnParticleChanged.AddDynamic(this, &UParticleSystemComponent::InitializeSystem);
	}

	InitializeSystem();

	// 템플릿이 유효하고 자동 활성화가 켜져있으면 활성화
	if (Template && bAutoActivate)
	{
		Activate(true);
	}
}

int32 UParticleSystemComponent::GetTotalActiveParticles() const
{
	int32 TotalCount = 0;
	for (int32 i = 0; i < EmitterInstances.Num(); i++)
	{
		TotalCount += GetActiveParticleCount(i);
	}
	return TotalCount;
}

int32 UParticleSystemComponent::GetActiveParticleCount(int32 EmitterIndex) const
{
	if ((0 <= EmitterIndex && EmitterIndex < EmitterInstances.Num()) && EmitterInstances[EmitterIndex])
	{
		return EmitterInstances[EmitterIndex]->ActiveParticles;
	}
	return 0;
}
