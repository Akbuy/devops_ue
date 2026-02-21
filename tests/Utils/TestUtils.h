// My game copyright

#pragma once

#if WITH_AUTOMATION_TESTS

#include "CoreMinimal.h"
#include "Tests/AutomationCommon.h"
#include "EnhancedInputComponent.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Blueprint/WidgetTree.h"

namespace LifeExe
{
namespace Test
{
class LevelScope
{
public:
    LevelScope(const FString& MapName) { AutomationOpenMap(MapName); }
    ~LevelScope() { ADD_LATENT_AUTOMATION_COMMAND(FExitGameCommand); }
};

template <typename Type1, typename Type2>
struct TestPayload
{
    Type1 TestValue;
    Type2 ExpectedValue;
    float Tolerance = KINDA_SMALL_NUMBER;
};

#define ENUM_LOOP_START(TYPE, EnumElem)                                        \
    for (int32 Index = 0; Index < StaticEnum<TYPE>()->NumEnums() - 1; ++Index) \
    {                                                                          \
        const auto EnumElem = static_cast<TYPE>(StaticEnum<TYPE>()->GetValueByIndex(Index));
#define ENUM_LOOP_END }

template <typename EnumType, typename FunctionType>
void ForEach(FunctionType&& Function)
{
    const UEnum* Enum = StaticEnum<EnumType>();
    for (int32 Index = 0; Index < Enum->NumEnums(); Index++)
    {
        Function(static_cast<EnumType>(Enum->GetValueByIndex(Index)), Enum->GetNameByIndex(Index));
    }
}

template <typename T>
T* CreateBlueprint(UWorld* World, const TCHAR* Name, const FTransform& Transform = FTransform::Identity)
{
    const UBlueprint* Blueprint = LoadObject<UBlueprint>(nullptr, Name);
    return (World && Blueprint) ? World->SpawnActor<T>(Blueprint->GeneratedClass, Transform) : nullptr;
}

template <typename T>
T* CreateBlueprintDeferred(UWorld* World, const TCHAR* Name, const FTransform& Transform = FTransform::Identity)
{
    const UBlueprint* Blueprint = LoadObject<UBlueprint>(nullptr, Name);
    return (World && Blueprint) ? World->SpawnActorDeferred<T>(Blueprint->GeneratedClass, Transform) : nullptr;
}

bool CallFuncByNameWithParams(UObject* Object, const FString& FuncName, const TArray<FString>& Params);

int32 GetActionBindingIndexByName(const UEnhancedInputComponent* InputComponent, const FString& ActionName, ETriggerEvent TriggerEvent);

FString GetTestDataDir();

template <class T>
T* FindWidgetByClass()
{
    const auto World = AutomationCommon::GetAnyGameWorld();
    if (!World) return nullptr;

    TArray<UUserWidget*> Widgets;
    UWidgetBlueprintLibrary::GetAllWidgetsOfClass(World, Widgets, T::StaticClass(), false);

    return Widgets.Num() != 0 ? Cast<T>(Widgets[0]) : nullptr;
}

UWidget* FindWidgetByName(const UUserWidget* Widget, const FName& Name);

void DoInputAction(UEnhancedInputComponent* InputComponent, const FString& ActionName, ETriggerEvent TriggerEvent);
void JumpPressed(UEnhancedInputComponent* InputComponent);
void PausePressed(UEnhancedInputComponent* InputComponent);

class FTakeScreenshotLatentCommand : public IAutomationLatentCommand
{
public:
    FTakeScreenshotLatentCommand(const FString& InScreenshotName);
    virtual ~FTakeScreenshotLatentCommand();

protected:
    const FString ScreenshotName;
    bool ScreenshotRequested{false};
    bool CommandCompleted{false};

    virtual void OnScreenshotTakenAndCompared();
};

class FTakeGameScreenshotLatentCommand : public FTakeScreenshotLatentCommand
{
public:
    FTakeGameScreenshotLatentCommand(const FString& InScreenshotName);

    virtual bool Update() override;
};

class FTakeUIScreenshotLatentCommand : public FTakeScreenshotLatentCommand
{
public:
    FTakeUIScreenshotLatentCommand(const FString& InScreenshotName);
    virtual bool Update() override;

private:
    bool ScreenshotSetupDone{false};

    virtual void OnScreenshotTakenAndCompared() override;
    void SetBufferVisualization(const FName& VisualizeBuffer);
};

void SpecCloseLevel(UWorld* World);

template <class ObjectClass, class PropertyClass>
PropertyClass GetPropertyValueByName(ObjectClass* Object, const FString& PropertyName)
{
    if (!Object) return PropertyClass();
    for (TFieldIterator<FProperty> PropIt(Object->StaticClass()); PropIt; ++PropIt)
    {
        const FProperty* Property = *PropIt;
        if (Property && Property->GetName().Equals(PropertyName))
        {
            return *Property->ContainerPtrToValuePtr<PropertyClass>(Object);
        }
    }
    return PropertyClass();
}

}  // namespace Test
}  // namespace LifeExe

#endif
