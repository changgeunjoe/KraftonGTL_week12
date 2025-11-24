#include "pch.h"

#include "ParticleModule.h"
#include "Source/Runtime/Core/Misc/JsonSerializer.h"


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
		// 다른 타입들도 필요시 추가...
		default:
			break;
		}
	}
}
