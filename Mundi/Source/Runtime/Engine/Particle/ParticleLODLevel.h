#pragma once
#include "UParticleLODLevel.generated.h"

#include "Source/Runtime/Core/Misc/JsonSerializer.h"

class UParticleEmitter;
class UParticleModuleTypeDataBase;
class UParticleModule;
class UParticleModuleSpawn;
class UParticleModuleSpawnBase;
class UParticleModuleRequired;

UCLASS()
class UParticleLODLevel : public UObject
{
	GENERATED_REFLECTION_BODY()

public:
	/** LOD 레벨의 인덱스 */
	int32 Level;

	/** LOD 레벨이 활성화되어있으면 True이며, 업데이트되고 렌더링되어야 함을 의미 */
	uint32 bEnabled : 1;

	/** 이 LOD 레벨에 의해 요구되는 모듈 */
	UParticleModuleRequired* RequiredModule;

	/** LOD 레벨에 대해 조정된 데이터를 포함하는 파티클 모듈의 배열 (Lifetime, Size, Velocity, Color, etc) */
	TArray<UParticleModule*> Modules;

	/** 이미터 타입 'extension'에 의해 사용됨 */
	UParticleModuleTypeDataBase* TypeDataModule;

	/** SpawnRate/Burst 모듈 - 모든 이미터들에 의해 요구됨 (생성 규칙 정의) */
	UParticleModuleSpawn* SpawnModule;

	/** SpawningModules - 얼마나 많은 파티클이 스폰될지 결정함 */
	TArray<UParticleModuleSpawnBase*> SpawningModules;

	/** SpawnModules - 파티클이 스폰될때 호출됨 (초기값 설정) */
	TArray<UParticleModule*> SpawnModules;

	/** UpdateModules - 파티클이 업데이트될때 호출됨 */
	TArray<UParticleModule*> UpdateModules;

	/** 이 LOD 레벨에서 예상되는 최대 활성 파티클 수 */
	int32 PeakActiveParticles;

	/** @note Outer 변수가 없어서 임의로 Owner 변수 추가 */
	UParticleEmitter* OwnerEmitter;

public:
	UParticleLODLevel();
	virtual ~UParticleLODLevel();

	//~Begin UObject Interface.

	/**
	 * JSON 직렬화/역직렬화
	 * @param bInIsLoading true면 로드, false면 저장
	 * @param InOutHandle JSON 데이터
	 */
	virtual void Serialize(const bool bInIsLoading, JSON& InOutHandle);

	//~End UObject Interface.

	/**
	 * @brief 파티클 생성 규칙(Rate, Burst)을 담당하는 SpawnModule을 추가/교체한다.
	 * 기존에 존재하던 모듈은 제거된다.
	 *
	 * @return 생성된 모듈의 포인터. 실패 시 nullptr.
	 */
	UParticleModuleSpawn* AddSpawnModule();

	/**
	 * LOD 레벨의 필수 모듈(UParticleModuleRequired)을 생성하거나 교체한다.
	 * 기존에 존재하던 모듈은 제거된다.
	 * @return 생성된 모듈의 포인터. 실패 시 nullptr
	 */
	UParticleModuleRequired* AddRequiredModule();

	/**
	 * 일반 속성 제어 모듈을 추가한다.
	 * SpawnModule이나 TypeDataModule은 이 함수로 추가할 수 없으며, 별도 함수를 사용해야 한다.
	 *
	 * @tparam TModule 생성할 모듈의 타입 정보
	 * @return 생성된 모듈의 포인터. 실패 시 nullptr.
	 */
	template <typename TModule>
	TModule* AddModule()
	{
		static_assert(std::is_base_of_v<UParticleModule, TModule>);
		return Cast<TModule>(AddModule(TModule::StaticClass()));
	}

	/**
	 * 일반 속성 제어 모듈을 추가한다.
	 * SpawnModule이나 TypeDataModule은 이 함수로 추가할 수 없으며, 별도 함수를 사용해야 한다.
	 *
	 * @param ModuleClass 생성할 모듈의 클래스 정보
	 * @return 생성된 모듈의 포인터. 실패 시 nullptr.
	 */
	UParticleModule* AddModule(UClass* ModuleClass);

	/** 모듈 리스트를 갱신하고 런타임 캐시 배열을 채운다. */
	virtual void UpdateModuleLists();

	/** 이 LOD 레벨의 최대 활성 파티클 수를 계산한다. */
	virtual int32 CalculateMaxActiveParticleCount();

};
