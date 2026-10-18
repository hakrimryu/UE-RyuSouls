#include "Animations/AnimNotifyState_RsComboWindow.h"

#include "Characters/RsCharacter.h"

UAnimNotifyState_RsComboWindow::UAnimNotifyState_RsComboWindow(const FObjectInitializer& ObjectInitializer)
{
}

void UAnimNotifyState_RsComboWindow::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                                 float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	// プレビューなど所有者がARsCharacterではない場合はコンボ処理を呼び出さない。
	if (ARsCharacter* Character = Cast<ARsCharacter>(MeshComp->GetOwner()))
	{
		Character->EnableComboWindow();
	}
}

void UAnimNotifyState_RsComboWindow::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	// 次段の再生判断はキャラクター側へ委ね、Notifyには入力状態を保持しない。
	if (ARsCharacter* Character = Cast<ARsCharacter>(MeshComp->GetOwner()))
	{
		Character->DisableComboWindow();
	}
}
