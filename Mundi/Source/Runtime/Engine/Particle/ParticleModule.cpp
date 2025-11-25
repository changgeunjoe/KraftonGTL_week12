#include "pch.h"

#include "ParticleModule.h"

#include "ParticleEmitterInstances.h"
#include "ParticleSystemComponent.h"
#include "Source/Runtime/Core/Misc/JsonSerializer.h"
#include "Source/Runtime/Engine/Distribution/Distributions.h"


UParticleModule::UParticleModule()
	: bSpawnModule(false)
	, bUpdateModule(false)
	, bFinalUpdateModule(false)
	, bEnabled(true)
{
}

void UParticleModule::Spawn(const FSpawnContext& Context)
{
}

void UParticleModule::Update(const FUpdateContext& Context)
{
}

void UParticleModule::FinalUpdate(const FUpdateContext& Context)
{
}

uint32 UParticleModule::RequiredBytes(UParticleModuleTypeDataBase* TypeData)
{
	return 0;
}

uint32 UParticleModule::RequiredBytesPerInstance()
{
	return 0;
}

uint32 UParticleModule::PrepPerInstanceBlock(FParticleEmitterInstance* Owner, void* InstData)
{
	return 0;
}

void UParticleModule::SetToSensibleDefaults(UParticleEmitter* Owner)
{
}

void UParticleModule::Serialize(const bool bInIsLoading, JSON& InOutHandle)
{
	// 클래스 타입 저장/로드
	if (!bInIsLoading)
	{
		InOutHandle["Type"] = GetClass()->Name;
	}

	// 리플렉션 시스템을 사용하여 모든 프로퍼티 자동 직렬화
	UClass* ModuleClass = GetClass();
	if (!ModuleClass)
		return;

	for (const FProperty& Prop : ModuleClass->Properties)
	{
		// 에디터 전용 프로퍼티는 제외
		if (!Prop.bIsEditAnywhere)
			continue;

		switch (Prop.Type)
		{
		case EPropertyType::Bool:
		{
			bool* ValuePtr = Prop.GetValuePtr<bool>(this);
			if (bInIsLoading)
				FJsonSerializer::ReadBool(InOutHandle, Prop.Name, *ValuePtr);
			else
				InOutHandle[Prop.Name] = *ValuePtr;
			break;
		}
		case EPropertyType::Int32:
		{
			int32* ValuePtr = Prop.GetValuePtr<int32>(this);
			if (bInIsLoading)
				FJsonSerializer::ReadInt32(InOutHandle, Prop.Name, *ValuePtr);
			else
				InOutHandle[Prop.Name] = *ValuePtr;
			break;
		}
		case EPropertyType::Float:
		{
			float* ValuePtr = Prop.GetValuePtr<float>(this);
			if (bInIsLoading)
				FJsonSerializer::ReadFloat(InOutHandle, Prop.Name, *ValuePtr);
			else
				InOutHandle[Prop.Name] = *ValuePtr;
			break;
		}
		case EPropertyType::FVector:
		{
			FVector* ValuePtr = Prop.GetValuePtr<FVector>(this);
			if (bInIsLoading)
			{
				FJsonSerializer::ReadVector(InOutHandle, Prop.Name, *ValuePtr);
			}
			else
			{
				JSON VecArray = JSON::Make(JSON::Class::Array);
				VecArray.append(ValuePtr->X);
				VecArray.append(ValuePtr->Y);
				VecArray.append(ValuePtr->Z);
				InOutHandle[Prop.Name] = VecArray;
			}
			break;
		}
		case EPropertyType::FLinearColor:
		{
			FLinearColor* ValuePtr = Prop.GetValuePtr<FLinearColor>(this);
			if (bInIsLoading)
			{
				JSON ColorArray;
				if (FJsonSerializer::ReadArray(InOutHandle, Prop.Name, ColorArray) && ColorArray.size() >= 4)
				{
					ValuePtr->R = ColorArray.at(0).ToFloat();
					ValuePtr->G = ColorArray.at(1).ToFloat();
					ValuePtr->B = ColorArray.at(2).ToFloat();
					ValuePtr->A = ColorArray.at(3).ToFloat();
				}
			}
			else
			{
				JSON ColorArray = JSON::Make(JSON::Class::Array);
				ColorArray.append(ValuePtr->R);
				ColorArray.append(ValuePtr->G);
				ColorArray.append(ValuePtr->B);
				ColorArray.append(ValuePtr->A);
				InOutHandle[Prop.Name] = ColorArray;
			}
			break;
		}
		case EPropertyType::FString:
		{
			FString* ValuePtr = Prop.GetValuePtr<FString>(this);
			if (bInIsLoading)
				FJsonSerializer::ReadString(InOutHandle, Prop.Name, *ValuePtr);
			else
				InOutHandle[Prop.Name] = *ValuePtr;
			break;
		}
		case EPropertyType::FName:
		{
			FName* ValuePtr = Prop.GetValuePtr<FName>(this);
			if (bInIsLoading)
			{
				FString TempString;
				FJsonSerializer::ReadString(InOutHandle, Prop.Name, TempString);
				*ValuePtr = FName(TempString);
			}
			else
			{
				InOutHandle[Prop.Name] = ValuePtr->ToString();
			}
			break;
		}
		case EPropertyType::Curve:
		{
			// Curve는 float[4]로 저장
			float* ValuePtr = Prop.GetValuePtr<float>(this);
			if (bInIsLoading)
			{
				JSON CurveArray;
				if (FJsonSerializer::ReadArray(InOutHandle, Prop.Name, CurveArray) && CurveArray.size() == 4)
				{
					for (int i = 0; i < 4; ++i)
						ValuePtr[i] = CurveArray.at(i).ToFloat();
				}
			}
			else
			{
				JSON CurveArray = JSON::Make(JSON::Class::Array);
				for (int i = 0; i < 4; ++i)
					CurveArray.append(ValuePtr[i]);
				InOutHandle[Prop.Name] = CurveArray;
			}
			break;
		}
		case EPropertyType::RawDistributionFloat:
		{
			FRawDistributionFloat* ValuePtr = Prop.GetValuePtr<FRawDistributionFloat>(this);
			if (bInIsLoading)
			{
				// Distribution 타입과 값 로드
				if (InOutHandle.hasKey(Prop.Name))
				{
					JSON& DistJson = InOutHandle[Prop.Name];
					FString DistType;
					FJsonSerializer::ReadString(DistJson, "DistributionType", DistType);

					// 기존 분포 삭제
					if (ValuePtr->Distribution)
					{
						DeleteObject(ValuePtr->Distribution);
						ValuePtr->Distribution = nullptr;
					}

					if (DistType == "Constant")
					{
						auto* ConstDist = NewObject<UDistributionFloatConstant>();
						FJsonSerializer::ReadFloat(DistJson, "Constant", ConstDist->Constant);
						ValuePtr->Distribution = ConstDist;
					}
					else if (DistType == "Uniform")
					{
						auto* UniformDist = NewObject<UDistributionFloatUniform>();
						FJsonSerializer::ReadFloat(DistJson, "Min", UniformDist->Min);
						FJsonSerializer::ReadFloat(DistJson, "Max", UniformDist->Max);
						ValuePtr->Distribution = UniformDist;
					}
				}
			}
			else
			{
				// Distribution 타입과 값 저장
				JSON DistJson = JSON::Make(JSON::Class::Object);
				if (ValuePtr->Distribution)
				{
					if (auto* ConstDist = Cast<UDistributionFloatConstant>(ValuePtr->Distribution))
					{
						DistJson["DistributionType"] = "Constant";
						DistJson["Constant"] = ConstDist->Constant;
					}
					else if (auto* UniformDist = Cast<UDistributionFloatUniform>(ValuePtr->Distribution))
					{
						DistJson["DistributionType"] = "Uniform";
						DistJson["Min"] = UniformDist->Min;
						DistJson["Max"] = UniformDist->Max;
					}
				}
				else
				{
					DistJson["DistributionType"] = "None";
				}
				InOutHandle[Prop.Name] = DistJson;
			}
			break;
		}
		case EPropertyType::RawDistributionVector:
		{
			FRawDistributionVector* ValuePtr = Prop.GetValuePtr<FRawDistributionVector>(this);
			if (bInIsLoading)
			{
				// Distribution 타입과 값 로드
				if (InOutHandle.hasKey(Prop.Name))
				{
					JSON& DistJson = InOutHandle[Prop.Name];
					FString DistType;
					FJsonSerializer::ReadString(DistJson, "DistributionType", DistType);

					// 기존 분포 삭제
					if (ValuePtr->Distribution)
					{
						DeleteObject(ValuePtr->Distribution);
						ValuePtr->Distribution = nullptr;
					}

					if (DistType == "Constant")
					{
						auto* ConstDist = NewObject<UDistributionVectorConstant>();
						FJsonSerializer::ReadVector(DistJson, "Constant", ConstDist->Constant);
						ValuePtr->Distribution = ConstDist;
					}
					else if (DistType == "Uniform")
					{
						auto* UniformDist = NewObject<UDistributionVectorUniform>();
						FJsonSerializer::ReadVector(DistJson, "Min", UniformDist->Min);
						FJsonSerializer::ReadVector(DistJson, "Max", UniformDist->Max);
						ValuePtr->Distribution = UniformDist;
					}
				}
			}
			else
			{
				// Distribution 타입과 값 저장
				JSON DistJson = JSON::Make(JSON::Class::Object);
				if (ValuePtr->Distribution)
				{
					if (auto* ConstDist = Cast<UDistributionVectorConstant>(ValuePtr->Distribution))
					{
						DistJson["DistributionType"] = "Constant";
						JSON VecArray = JSON::Make(JSON::Class::Array);
						VecArray.append(ConstDist->Constant.X);
						VecArray.append(ConstDist->Constant.Y);
						VecArray.append(ConstDist->Constant.Z);
						DistJson["Constant"] = VecArray;
					}
					else if (auto* UniformDist = Cast<UDistributionVectorUniform>(ValuePtr->Distribution))
					{
						DistJson["DistributionType"] = "Uniform";
						JSON MinArray = JSON::Make(JSON::Class::Array);
						MinArray.append(UniformDist->Min.X);
						MinArray.append(UniformDist->Min.Y);
						MinArray.append(UniformDist->Min.Z);
						DistJson["Min"] = MinArray;

						JSON MaxArray = JSON::Make(JSON::Class::Array);
						MaxArray.append(UniformDist->Max.X);
						MaxArray.append(UniformDist->Max.Y);
						MaxArray.append(UniformDist->Max.Z);
						DistJson["Max"] = MaxArray;
					}
				}
				else
				{
					DistJson["DistributionType"] = "None";
				}
				InOutHandle[Prop.Name] = DistJson;
			}
			break;
		}
		case EPropertyType::Material:
		{
			// UMaterialInterface* 타입 직렬화 - 파일 경로로 저장/로드
			UMaterialInterface** ValuePtr = Prop.GetValuePtr<UMaterialInterface*>(this);
			if (bInIsLoading)
			{
				FString MaterialPath;
				FJsonSerializer::ReadString(InOutHandle, Prop.Name, MaterialPath);
				if (!MaterialPath.empty())
				{
					*ValuePtr = UResourceManager::GetInstance().Load<UMaterial>(MaterialPath);
				}
				else
				{
					*ValuePtr = nullptr;
				}
			}
			else
			{
				if (*ValuePtr && !(*ValuePtr)->GetFilePath().empty())
				{
					InOutHandle[Prop.Name] = (*ValuePtr)->GetFilePath();
				}
				else
				{
					InOutHandle[Prop.Name] = "";
				}
			}
			break;
		}
		case EPropertyType::StaticMesh:
		{
			// UStaticMesh* 타입 직렬화 - 파일 경로로 저장/로드
			UStaticMesh** ValuePtr = Prop.GetValuePtr<UStaticMesh*>(this);
			if (bInIsLoading)
			{
				FString MeshPath;
				FJsonSerializer::ReadString(InOutHandle, Prop.Name, MeshPath);
				if (!MeshPath.empty())
				{
					*ValuePtr = UResourceManager::GetInstance().Load<UStaticMesh>(MeshPath);
				}
				else
				{
					*ValuePtr = nullptr;
				}
			}
			else
			{
				if (*ValuePtr && !(*ValuePtr)->GetFilePath().empty())
				{
					InOutHandle[Prop.Name] = (*ValuePtr)->GetFilePath();
				}
				else
				{
					InOutHandle[Prop.Name] = "";
				}
			}
			break;
		}
		case EPropertyType::Texture:
		{
			// UTexture* 타입 직렬화 - 파일 경로로 저장/로드
			UTexture** ValuePtr = Prop.GetValuePtr<UTexture*>(this);
			if (bInIsLoading)
			{
				FString TexturePath;
				FJsonSerializer::ReadString(InOutHandle, Prop.Name, TexturePath);
				if (!TexturePath.empty())
				{
					*ValuePtr = UResourceManager::GetInstance().Load<UTexture>(TexturePath);
				}
				else
				{
					*ValuePtr = nullptr;
				}
			}
			else
			{
				if (*ValuePtr && !(*ValuePtr)->GetFilePath().empty())
				{
					InOutHandle[Prop.Name] = (*ValuePtr)->GetFilePath();
				}
				else
				{
					InOutHandle[Prop.Name] = "";
				}
			}
			break;
		}
		case EPropertyType::Enum:
		{
			// Enum 타입 직렬화 - int32로 저장/로드
			int32* ValuePtr = Prop.GetValuePtr<int32>(this);
			if (bInIsLoading)
				FJsonSerializer::ReadInt32(InOutHandle, Prop.Name, *ValuePtr);
			else
				InOutHandle[Prop.Name] = *ValuePtr;
			break;
		}
		// 다른 타입들도 필요시 추가...
		default:
			break;
		}
	}
}

const FTransform& UParticleModule::FContext::GetTransform() const
{
	return Owner.Component->GetWorldTransform();
}

UObject* UParticleModule::FContext::GetDistributionData() const
{
	// @todo 언리얼엔진의 컨벤션을 따르기 위한 함수, 현재는 구현 없음
	return nullptr;
}

FString UParticleModule::FContext::GetTemplateName() const
{
	// @todo 언리얼엔진의 컨벤션을 따르기 위한 함수, 현재는 구현 없음
	return "";
}

FString UParticleModule::FContext::GetInstanceName() const
{
	// @todo 언리얼엔진의 컨벤션을 따르기 위한 함수, 현재는 구현 없음
	return "";
}
