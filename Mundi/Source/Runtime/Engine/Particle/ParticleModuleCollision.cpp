#include "pch.h"
#include "ParticleModuleCollision.h"
#include "ParticleEmitterInstances.h"
#include "ParticleSystemComponent.h"
#include "ParticleHelper.h" // STATE_Particle_Freeze 등의 플래그 정의 필요

UParticleModuleCollision::UParticleModuleCollision()
{
	bSpawnModule = true;
	bUpdateModule = true;
	bEnabled = true;

	DirScalar = 1.0f;
	bKillOnMaxCollision = true;

	InitializeDefaults();
}

void UParticleModuleCollision::InitializeDefaults()
{
	if (!DampingFactor.IsCreated())
	{
		auto* Dist = NewObject<UDistributionVectorConstant>();
		Dist->Constant = FVector(0.5f, 0.5f, 0.5f);
		DampingFactor.Distribution = Dist;
	}

	if (!MaxCollisions.IsCreated())
	{
		auto* Dist = NewObject<UDistributionFloatConstant>();
		Dist->Constant = 1.0f;
		MaxCollisions.Distribution = Dist;
	}

	if (!DelayAmount.IsCreated())
	{
		auto* Dist = NewObject<UDistributionFloatConstant>();
		Dist->Constant = 0.0f;
		DelayAmount.Distribution = Dist;
	}
}

uint32 UParticleModuleCollision::RequiredBytes(UParticleModuleTypeDataBase* TypeData)
{
	return sizeof(FParticleCollisionPayload);
}

void UParticleModuleCollision::Spawn(const FSpawnContext& Context)
{
	SPAWN_INIT;

	PARTICLE_ELEMENT(FParticleCollisionPayload, Payload);

	Payload.UsedDampingFactor = DampingFactor.GetValue(Particle.RelativeTime, Context.GetDistributionData());
	Payload.UsedCollisions = std::round(MaxCollisions.GetValue(Particle.RelativeTime, Context.GetDistributionData()));
	Payload.Delay = DelayAmount.GetValue(Particle.RelativeTime, Context.GetDistributionData());
}

void UParticleModuleCollision::Update(const FUpdateContext& Context)
{
	FParticleEmitterInstance* Owner = &Context.Owner;
	UParticleSystemComponent* Component = Owner->Component;

	if (!Component || Owner->ActiveParticles <= 0)
	{
		return;
	}

	AActor* Actor = Component->GetOwner();

	const bool bKill = bKillOnMaxCollision;
	const float Scalar = DirScalar;

	BEGIN_UPDATE_LOOP;
	{
		PARTICLE_ELEMENT(FParticleCollisionPayload, Payload);

		// 1. 딜레이 체크
		if (Payload.Delay > Particle.RelativeTime)
		{
			continue;
		}

		// 2. 물리 시뮬레이션 (예상 이동 위치 계산)
		FVector OldLocation = Particle.Location;
		FVector Velocity = (FVector)Particle.Velocity;
		FVector Location = OldLocation + (Velocity * Context.DeltaTime);

		FVector Direction = (Location - OldLocation);
		float Dist = Direction.Size();

		if (Dist < KINDA_SMALL_NUMBER)
		{
			continue;
		}

		FVector DirNormal = Direction / Dist;

		// 3. 충돌 검사 범위 설정 (Size 고려)
		float MaxSize = FMath::Max(Particle.Size.X, Particle.Size.Y);
		FVector Extent(MaxSize * 0.5f);

		FVector CheckEnd = Location + (DirNormal * (MaxSize * Scalar));

		FHitResult Hit;

		// 4. 충돌 검사 수행
		if (PerformCollisionCheck(Owner, &Particle, Hit, Actor, CheckEnd, OldLocation, Extent))
		{
			if (Payload.UsedCollisions > 0)
			{
				Payload.UsedCollisions--;

				FVector ReflectedDir = DirNormal - Hit.Normal * (2.f * FVector::Dot(DirNormal, Hit.Normal));

				FVector NewBaseVelocity = ReflectedDir * Velocity.Size() * (FVector)Payload.UsedDampingFactor;

				Particle.BaseVelocity = NewBaseVelocity;

				Particle.Velocity = FVector::Zero();
				Particle.Location = Hit.Location + (NewBaseVelocity * (1.f - Hit.Time) * Context.DeltaTime);

				Particle.Location += Hit.Normal * 0.1f;
			}
			else
			{
				if (bKill)
				{
					Particle.RelativeTime = 1.1f;
				}
				else
				{
					Particle.Location = Hit.Location + Hit.Normal * 0.1f;
					Particle.Velocity = FVector::Zero();
					Particle.BaseVelocity = FVector::Zero();
				}
			}
		}
	}
	END_UPDATE_LOOP;
}

bool UParticleModuleCollision::PerformCollisionCheck(FParticleEmitterInstance* Owner, FBaseParticle* InParticle, FHitResult& Hit, AActor* SourceActor, const FVector& End, const FVector& Start, const FVector& Extent)
{
	if (Owner && Owner->Component)
	{
		return Owner->Component->ParticleLineCheck(Hit, SourceActor, End, Start, Extent);
	}
	return false;
}
