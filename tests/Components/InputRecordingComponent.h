// My game copyright

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "<Path>/Tests/Utils/InputRecordingUtils.h"
#include "EnhancedInputComponent.h"
#include "InputRecordingComponent.generated.h"

class UPlayerInput;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class MY_GAME_API UInputRecordingComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UInputRecordingComponent();

protected:
    virtual void BeginPlay() override;

    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
    FString FileName{"CharacterTestInput"};

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    FInputData RecordedInputData;

    FBindingsData MakeBindingsData() const;
    FString GenerateFileName() const;
};
