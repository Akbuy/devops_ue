// My game copyright

#pragma once

#include "CoreMinimal.h"
#include "InputAction.h"
#include "InputRecordingUtils.generated.h"

USTRUCT()
struct FActionData
{
    GENERATED_BODY()

    UPROPERTY()
    FName Name;

    UPROPERTY()
    FVector Value;

public:
    FActionData() = default;
    FActionData(const FInputActionValue& InSourceActionValue) : Value(InSourceActionValue.Get<FVector>()) {}
    FActionData(FName Name, const FInputActionValue& InSourceActionValue) : Name(Name), Value(InSourceActionValue.Get<FVector>()) {}
};

USTRUCT()
struct FBindingsData
{
    GENERATED_BODY()

    UPROPERTY()
    TArray<FActionData> ActionValues;

    UPROPERTY()
    float WorldTime{0.0f};
};

USTRUCT()
struct FInputData
{
    GENERATED_BODY()

    UPROPERTY()
    TArray<FBindingsData> Bindings;

    UPROPERTY()
    FTransform InitialTransform;
};