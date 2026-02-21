// My game copyright

#if WITH_AUTOMATION_TESTS

#include "TestUtils.h"
#include "Misc/OutputDeviceNull.h"
#include "AutomationBlueprintFunctionLibrary.h"  // add FunctionalTesting to PublicDependencyModuleNames in your .Build.cs file
#include "BufferVisualizationData.h"

namespace LifeExe
{
namespace Test
{

bool CallFuncByNameWithParams(UObject* Object, const FString& FuncName, const TArray<FString>& Params)
{
    if (!Object) return false;

    FString Command = FString::Printf(TEXT("%s"), *FuncName);
    for (const FString& Param : Params)
    {
        Command.Append(" ").Append(Param);
    }
    FEditorScriptExecutionGuard ScriptGuard;
    FOutputDeviceNull OutputDeviceNull;
    return Object->CallFunctionByNameWithArguments(*Command, OutputDeviceNull, nullptr, true);
}
int32 GetActionBindingIndexByName(const UEnhancedInputComponent* InputComponent, const FString& ActionName, ETriggerEvent TriggerEvent)
{
    if (!InputComponent) return INDEX_NONE;

    const int32 ActionIndex = InputComponent->GetActionEventBindings().IndexOfByPredicate(  //
        [ActionName, TriggerEvent](const TUniquePtr<FEnhancedInputActionEventBinding>& ActionBinding)
        {
            return ActionBinding->GetAction()->GetName().Equals(ActionName)  //
                   && ActionBinding->GetTriggerEvent() == TriggerEvent;
        });

    return ActionIndex;
}
FString GetTestDataDir()
{
    return FPaths::GameSourceDir().Append("<Path-to-test>/Tests/Data/");
}

UWidget* FindWidgetByName(const UUserWidget* Widget, const FName& Name)
{
    if (!Widget || !Widget->WidgetTree) return nullptr;

    UWidget* FoundWidget = nullptr;
    UWidgetTree::ForWidgetAndChildren(Widget->WidgetTree->RootWidget,
        [&](UWidget* Child)
        {
            if (Child && Child->GetFName().IsEqual(Name))
            {
                FoundWidget = Child;
            }
        });
    return FoundWidget;
}

void DoInputAction(UEnhancedInputComponent* InputComponent, const FString& ActionName, ETriggerEvent TriggerEvent)
{
    if (!InputComponent) return;

    const int32 ActionIndex = GetActionBindingIndexByName(InputComponent, ActionName, TriggerEvent);
    if (ActionIndex != INDEX_NONE)
    {
        const auto& ActionBind = InputComponent->GetActionEventBindings()[ActionIndex];
        if (!ActionBind) return;

        ActionBind->Execute(ActionBind->GetAction());
    }
}

void JumpPressed(UEnhancedInputComponent* InputComponent)
{
    DoInputAction(InputComponent, "IA_Jump", ETriggerEvent::Started);
}
void PausePressed(UEnhancedInputComponent* InputComponent)
{
    DoInputAction(InputComponent, "IA_Pause", ETriggerEvent::Started);
}

FTakeScreenshotLatentCommand::FTakeScreenshotLatentCommand(const FString& InScreenshotName) : ScreenshotName(InScreenshotName)
{
    FAutomationTestFramework::Get().OnScreenshotTakenAndCompared.AddRaw(this, &FTakeScreenshotLatentCommand::OnScreenshotTakenAndCompared);
}

FTakeScreenshotLatentCommand::~FTakeScreenshotLatentCommand()
{
    FAutomationTestFramework::Get().OnScreenshotTakenAndCompared.RemoveAll(this);
}

void FTakeScreenshotLatentCommand::OnScreenshotTakenAndCompared()
{
    CommandCompleted = true;
}

FTakeGameScreenshotLatentCommand::FTakeGameScreenshotLatentCommand(const FString& InScreenshotName)
    : FTakeScreenshotLatentCommand(InScreenshotName)
{
}

bool FTakeGameScreenshotLatentCommand::Update()
{
    if (!ScreenshotRequested)
    {
        const auto World = AutomationCommon::GetAnyGameWorld();
        auto Options = UAutomationBlueprintFunctionLibrary::GetDefaultScreenshotOptionsForRendering();
        Options.Resolution = FVector2D(1920, 1080);
        UAutomationBlueprintFunctionLibrary::TakeAutomationScreenshotInternal(World, ScreenshotName, FString{}, Options);
        ScreenshotRequested = true;
    }
    return CommandCompleted;
}

FTakeUIScreenshotLatentCommand::FTakeUIScreenshotLatentCommand(const FString& InScreenshotName)
    : FTakeScreenshotLatentCommand(InScreenshotName)
{
}

bool FTakeUIScreenshotLatentCommand::Update()
{
    if (!ScreenshotSetupDone)
    {
        SetBufferVisualization("Opacity");
        const double Duration = 1.0f;
        const double NewTime = FPlatformTime::Seconds();
        if (NewTime - StartTime >= Duration)
        {
            ScreenshotSetupDone = true;
        }
        return false;
    }

    if (!ScreenshotRequested)
    {
        const auto World = AutomationCommon::GetAnyGameWorld();
        auto Options = UAutomationBlueprintFunctionLibrary::GetDefaultScreenshotOptionsForRendering();
        Options.Resolution = FVector2D(1920, 1080);
        UAutomationBlueprintFunctionLibrary::TakeAutomationScreenshotOfUI_Immediate(World, ScreenshotName, Options);
        ScreenshotRequested = true;
    }
    return CommandCompleted;
}

void FTakeUIScreenshotLatentCommand::OnScreenshotTakenAndCompared()
{
    FTakeScreenshotLatentCommand::OnScreenshotTakenAndCompared();
    SetBufferVisualization(NAME_None);
}

void FTakeUIScreenshotLatentCommand::SetBufferVisualization(const FName& VisualizeBuffer)
{
    if (UGameViewportClient* ViewportClient = GEngine->GameViewport)
    {
        static IConsoleVariable* ICVar =
            IConsoleManager::Get().FindConsoleVariable(FBufferVisualizationData::GetVisualizationTargetConsoleCommandName());
        if (ICVar)
        {
            if (ViewportClient->GetEngineShowFlags())
            {
                ViewportClient->GetEngineShowFlags()->SetVisualizeBuffer(VisualizeBuffer == NAME_None ? false : true);
                ViewportClient->GetEngineShowFlags()->SetTonemapper(VisualizeBuffer == NAME_None ? true : false);
                ICVar->Set(VisualizeBuffer == NAME_None ? TEXT("") : *VisualizeBuffer.ToString());
            }
        }
    }
}

void SpecCloseLevel(UWorld* World)
{
    if (APlayerController* PlayerController = World->GetFirstPlayerController())
    {
        PlayerController->ConsoleCommand(TEXT("Exit"), true);
    }
}

}  // namespace Test
}  // namespace LifeExe

#endif