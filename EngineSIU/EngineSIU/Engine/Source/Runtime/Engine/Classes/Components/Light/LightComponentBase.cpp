#include "LightComponentBase.h"
#include "UObject/Casts.h"

ULightComponentBase::ULightComponentBase()
{
    Intensity = 1.0f;
    LightColor = FColor::White;

    bCastShadows = true;
}

ULightComponentBase::~ULightComponentBase()
{
  
}

void ULightComponentBase::GetProperties(TMap<FString, FString>& OutProperties) const
{
    USceneComponent::GetProperties(OutProperties);

    OutProperties.Add(TEXT("Intensity"), FString::Printf(TEXT("%f"), GetIntensity()));
    OutProperties.Add(TEXT("LightColor"), FLinearColor::FromColor(LightColor).ToString());
}

void ULightComponentBase::SetProperties(const TMap<FString, FString>& InProperties)
{
    USceneComponent::SetProperties(InProperties);
    const FString* TempStr = nullptr;

    TempStr = InProperties.Find(TEXT("Intensity"));
    if (TempStr)
    {
        Intensity = FCString::Atof(**TempStr++);
    }

    TempStr = InProperties.Find(TEXT("LightColor"));
    if (TempStr)
    {
        LightColor = FLinearColor(*TempStr).ToColorSRGB();
    }
    
}

UObject* ULightComponentBase::Duplicate(UObject* InOuter)
{
    ThisClass* NewComponent = Cast<ThisClass>(Super::Duplicate(InOuter));

    if (!NewComponent)
        return nullptr;
    
    NewComponent->Intensity = Intensity;
    NewComponent->LightColor = LightColor;

    return NewComponent;
}

