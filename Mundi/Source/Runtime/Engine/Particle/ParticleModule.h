#pragma once

#include "Source/Runtime/Core/Misc/JsonSerializer.h"

#include "UParticleModule.generated.h"
class UParticleEmitter;
class UParticleModuleTypeDataBase;
struct FBaseParticle;
struct FParticleEmitterInstance;

enum EModuleType : int
{
	EPMT_General,
	EPMT_TypeData,
	EPMT_Beam,
	EPMT_Trail,
	EPMT_Spawn,
	EPMT_Required,
	EPMT_Event,
	EPMT_Light,
	EPMT_SubUV,
	EPMT_MAX
};

/**
 * 파티클 모듈의 기본 클래스.
 * 모든 파티클 모듈은 이 클래스를 상속받아 구현된다.
 */
UCLASS(DisplayName="Particle Module", Description="파티클 모듈의 기본 클래스")
class UParticleModule : public UObject
{
	GENERATED_REFLECTION_BODY()
public:
	/** true일 경우, 모듈은 스폰 동안 파티클에 대하여 연산을 한다. */
	uint8 bSpawnModule:1;

	/** true일 경우, 모듈은 업데이트 동안 파티클에 대하여 연산을 한다. */
	uint8 bUpdateModule:1;

	/** true일 경우, 모듈은 마지막 업데이트 동안 파티클에 대하여 연산을 한다. */
	uint8 bFinalUpdateModule:1;

	/** true일 경우, 모듈은 활성화된다. */
	uint8 bEnabled:1;

	/** @note Outer 변수가 없어서 임의로 Owner 변수 추가 */
	UParticleEmitter* OwnerEmitter;

public:
	UParticleModule();

	virtual ~UParticleModule() = default;

	//~Begin UObject Interface.

	/**
	 * JSON 직렬화/역직렬화
	 * @param bInIsLoading true면 로드, false면 저장
	 * @param InOutHandle JSON 데이터
	 */
	virtual void Serialize(const bool bInIsLoading, JSON& InOutHandle);

	//~End UObject Interface.

	struct FContext
	{
		FParticleEmitterInstance& Owner;
		const FTransform& GetTransform() const;
		UObject* GetDistributionData() const;
		FString GetTemplateName() const;
		FString GetInstanceName() const;
		FContext(FParticleEmitterInstance& Ow) : Owner(Ow) {}
	};

	/**
	 * 이미터에 의해 갓 스폰된 파티클에 대해 호출된다. (순수 가상 함수이므로 자식들에 의해 구현되어야 함)
	 *
	 * @params Owner		파티클을 스폰한 FParticleEmitterInstance
	 * @params Offset		파티클의 데이터 페이로드에 들어가는 모듈의 오프셋
	 * @params SpawnTime	스폰된 시간
	 */
	struct FSpawnContext : FContext
	{
		int32 Offset;
		float SpawnTime;
		FBaseParticle* ParticleBase;
		FSpawnContext(FParticleEmitterInstance& Ow, int32 Of, float St, FBaseParticle* Pb) : FContext(Ow), Offset(Of), SpawnTime(St), ParticleBase(Pb) {}
	};
	virtual void Spawn(const FSpawnContext& Context);

	/**
	 * 이미터에 의해 업데이트되는 파티클에 대해 호출된다. (순수 가상 함수이므로 자식들에 의해 구현되어야 함)
	 *
	 * @params Owner		파티클을 스폰한 FParticleEmitterInstance
	 * @params Offset		파티클의 데이터 페이로드에 들어가는 모듈의 오프셋
	 * @params SpawnTime	스폰된 시간
	 */
	struct FUpdateContext : FContext
	{
		int32 Offset;
		float DeltaTime;
		FUpdateContext(FParticleEmitterInstance& Ow, int32 Of, float Dt) : FContext(Ow), Offset(Of), DeltaTime(Dt) {}
	};
	virtual void Update(const FUpdateContext& Context);

	/**
	 * 모든 업데이트 연산이 끝난 이미터에 대해 호출된다. (순수 가상 함수이므로 자식들에 의해 구현되어야 함)
	 *
	 * @params Owner		파티클을 스폰한 FParticleEmitterInstance
	 * @params Offset		파티클의 데이터 페이로드에 들어가는 모듈의 오프셋
	 * @params SpawnTime	스폰된 시간
	 */
	virtual void FinalUpdate(const FUpdateContext& Context);

	/**
	 * 모듈의 모듈 타입을 반환한다.
	 * @return EModuleType	모듈의 타입
	 */
	virtual EModuleType GetModuleType() const { return EPMT_General; }

	/**
	 * 모듈이 파티클 페이로드 블록에서 요구하는 바이트 수를 반환한다.
	 *
	 * @param TypeData		이 모듈이 가지고있는 이미터에 대한 UParticleModuleTypeDataBase
	 *
	 * @return uint32		파티클당 모듈이 필요로하는 바이트 수
	 */
	virtual uint32 RequiredBytes(UParticleModuleTypeDataBase* TypeData);

	/**
	 * 모듈이 요구하는 '인스턴스 당 데이터 블록'의 바이트 수를 반환한다.
	 * @return uint32		이미터 인스턴스 당 모듈이 필요로 하는 바이트 수
	 */
	virtual uint32 RequiredBytesPerInstance();
	/**
	 * 모듈이 자신의 '인스턴스 당(per-instance)' 데이터 블록을 준비하도록 한다.
	 *
	 * @param Owner			이 파티클을 소유하는 FParticleEmitterInsatnce
	 * @param InstData		이 모듈을 위한 데이터 블록의 포인터
	 * @return
	 */
	virtual uint32 PrepPerInstanceBlock(FParticleEmitterInstance* Owner, void* InstData);

	/**
	 * 모듈이 생성될때 호출되어서, 값들을 적절한 디폴트 값으로 초기화한다.
	 *
	 * @param Owner			모듈이 추가될 UParticleEmitter
	 */
	virtual void SetToSensibleDefaults(UParticleEmitter* Owner);
};


