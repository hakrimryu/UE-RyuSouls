#include "Component/RsStateComponent.h"

#include "Kismet/KismetSystemLibrary.h"

URsStateComponent::URsStateComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void URsStateComponent::BeginPlay()
{
	Super::BeginPlay();
}

void URsStateComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void URsStateComponent::ToggleMovementInput(bool bEnable, float Duration)
{
	if (bEnable)
	{
		// このコンポーネントのUFUNCTIONを遅延完了時の呼び出し先にする。
		FLatentActionInfo LatentAction;
		LatentAction.CallbackTarget = this;
		LatentAction.ExecutionFunction = "MovementInputEnableAction";
		LatentAction.Linkage = 0;
		// 同じCallbackTargetとUUIDを使い、復帰要求が重なると同じ遅延の待ち時間をリセットする。
		LatentAction.UUID = 0;

		// Durationは秒単位。0秒でもここで直接復帰せず、遅延処理の完了に委ねる。
		UKismetSystemLibrary::RetriggerableDelay(GetWorld(), Duration, LatentAction);
	}
	else
	{
		// 入力の禁止だけを行う。すでに予約された復帰の遅延処理は取り消さない。
		bMoveInputEnabled = false;
	}
}

void URsStateComponent::MovementInputEnableAction()
{
	// 入力の復帰と同時にローリングなどの行動状態を解除する。
	bMoveInputEnabled = true;
	ClearState();
}

void URsStateComponent::ClearState()
{
	CurrentState = FGameplayTag::EmptyTag;
}

bool URsStateComponent::IsCurrentStateEqualToAny(const FGameplayTagContainer& TagsToCheck) const
{
	return TagsToCheck.HasTagExact(CurrentState);
}
