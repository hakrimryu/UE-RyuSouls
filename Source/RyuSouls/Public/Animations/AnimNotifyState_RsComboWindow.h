#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "AnimNotifyState_RsComboWindow.generated.h"

// Montageの指定区間をコンボの入力受付期間としてキャラクターへ通知する。
UCLASS()
class RYUSOULS_API UAnimNotifyState_RsComboWindow : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	UAnimNotifyState_RsComboWindow(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	// 区間開始時に追加入力の受付を有効にする。
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	// 区間終了時に受付を閉じ、予約入力があれば次の攻撃へ進める。
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
