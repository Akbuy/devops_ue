// My game copyright

#include "<Path>/Tests/Components/InputRecordingComponent.h"
#include "EnhancedInputComponent.h"
#include "Engine/World.h"
#include "<Path>/Tests/Utils/JsonUtils.h"

using namespace LifeExe::Test;

UInputRecordingComponent::UInputRecordingComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UInputRecordingComponent::BeginPlay()
{
    Super::BeginPlay();

    check(GetOwner());
    check(GetOwner()->InputComponent);
    check(GetWorld());

    UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(GetOwner()->InputComponent);
    check(EnhancedInputComponent);

    const auto& ActionEventBindings = EnhancedInputComponent->GetActionEventBindings();
    for (const auto& ActionEventBinding : ActionEventBindings)
    {
        EnhancedInputComponent->BindActionValue(ActionEventBinding->GetAction());
    }

    RecordedInputData.InitialTransform = GetOwner()->GetActorTransform();
    RecordedInputData.Bindings.Add(MakeBindingsData());
}

void UInputRecordingComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);

    JsonUtils::WriteInputData(GenerateFileName(), RecordedInputData);
}

void UInputRecordingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    RecordedInputData.Bindings.Add(MakeBindingsData());
}

FBindingsData UInputRecordingComponent::MakeBindingsData() const
{
    FBindingsData BindingsData;
    BindingsData.WorldTime = GetWorld()->GetTimeSeconds();

    UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(GetOwner()->InputComponent);
    if (!EnhancedInputComponent) return BindingsData;

    TArray<FEnhancedInputActionValueBinding> ActionValueBindings = EnhancedInputComponent->GetActionValueBindings();
    for (const FEnhancedInputActionValueBinding ActionValueBinding : ActionValueBindings)
    {
        BindingsData.ActionValues.Add(FActionData{ActionValueBinding.GetAction()->GetFName(), ActionValueBinding.GetValue()});
    }

    return BindingsData;
}

FString UInputRecordingComponent::GenerateFileName() const
{
    FString SavedDir = FPaths::ProjectSavedDir();
    const FString Date = FDateTime::Now().ToString();
    return SavedDir.Append("/Tests/").Append(FileName).Append("_").Append(Date).Append(".json");
}
