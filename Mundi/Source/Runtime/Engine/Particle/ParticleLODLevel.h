#pragma once

class UParticleModuleTypeDataBase;
class UParticleModule;
class UParticleModuleSpawn;
class UParticleModuleSpawnBase;
class UParticleModuleRequired;

UCLASS()
class UParticleLODLevel : public UObject
{
	DECLARE_CLASS(UParticleLODLevel, UObject)

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

public:
	UParticleLODLevel();
	virtual ~UParticleLODLevel() = default;

	//~Begin UObject Interface.

	// Serialize...

	//~End UObject Interface.

	/** 모듈 리스트를 갱신하고 런타임 캐시 배열을 채운다. */
	virtual void UpdateModuleLists();

	/** 이 LOD 레벨의 최대 활성 파티클 수를 계산한다. */
	virtual int32 CalculateMaxActiveParticleCount();
};
