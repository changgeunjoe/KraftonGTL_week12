#pragma once
#include "UParticleSystem.generated.h"

#include "Source/Runtime/Core/Misc/JsonSerializer.h"

class UParticleEmitter;
class UParticleSystemComponent;

UCLASS()
class UParticleSystem : public UObject
{
	GENERATED_REFLECTION_BODY()

public:
	/** 시스템 내에 존재하는 이미터들의 배열 */
	TArray<UParticleEmitter*> Emitters;

	/** 파티클 시스템을 위한 바운딩 박스 (언리얼엔진에서는 FBox 타입을 사용) */
	FAABB FixedRelativeBoundingBox;

public:
	UParticleSystem() = default;

	virtual ~UParticleSystem();

	//~Begin UObject Interface.

	/**
	 * JSON 직렬화/역직렬화
	 * @param bInIsLoading true면 로드, false면 저장
	 * @param InOutHandle JSON 데이터
	 */
	virtual void Serialize(const bool bInIsLoading, JSON& InOutHandle);

	/**
	 * 파일에서 파티클 시스템 로드
	 * @param FilePath .particle 파일 경로
	 * @return 성공 여부
	 */
	bool LoadFromFile(const FWideString& FilePath);

	/**
	 * 파일로 파티클 시스템 저장
	 * @param FilePath .particle 파일 경로
	 * @return 성공 여부
	 */
	bool SaveToFile(const FWideString& FilePath);

	/**
	 * ResourceManager용 Load 인터페이스
	 * @param FilePath .particle 파일 경로
	 * @param Device D3D11 디바이스 (사용하지 않음, 인터페이스 호환성)
	 */
	void Load(const FString& FilePath, ID3D11Device* Device = nullptr);

	//~End UObject Interface.

	/**
	 * 새로운 이미터를 생성하고 시스템에 추가한다.
	 * 기본 LOD(0)와 RequiredModule, SpawnModule까지 디폴트로 생성하여 세팅한다.
	 * @tparam TEmitter		생성할 이미터 타입
	 * @return				생성된 이미터의 포인터
	 */
	template<typename TEmitter>
	TEmitter* AddEmitter()
	{
		static_assert(std::is_base_of<UParticleEmitter, TEmitter>());
		return AddEmitter(TEmitter::StaticClass());
	}

	/**
	 * 새로운 이미터를 생성하고 시스템에 추가한다.
	 * 기본 LOD(0)와 RequiredModule, SpawnModule까지 디폴트로 생성하여 세팅한다.
	 * @param EmitterClass	생성할 이미터 타입
	 * @return				생성된 이미터의 포인터
	 */
	UParticleEmitter* AddEmitter(UClass* EmitterClass);

	/**
	 * 각 이미터에 대하여 최대 활성 파티클 수를 결정한다. 이미터의 생명주기 동안 재할당을 피하기 위해 사용한다.
	 *
	 * @return true		각 이미터들에 대하여 숫자가 결정될 때
	 *		   false	결정되지 않을 때
	 */
	virtual bool CalculateMaxActiveParticleCounts();

	/**
	 * 모든 이미터 모듈 리스트를 업데이트한다.
	 */
	void UpdateAllModuleLists();
};
