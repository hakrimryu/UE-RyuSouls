#include "Animations/AnimNotify_RsAttackFinished.h"

#include "Characters/RsCharacter.h"

UAnimNotify_RsAttackFinished::UAnimNotify_RsAttackFinished(const FObjectInitializer& ObjectInitializer)
{
}

void UAnimNotify_RsAttackFinished::Notify(USkeletalMeshComponent* MeshComponent, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComponent, Animation, EventReference);

	// プレビューなど所有者がARsCharacterではない場合は、移動やタイマーを操作しない。
	if (ARsCharacter* Character = Cast<ARsCharacter>(MeshComponent->GetOwner()))
	{
		Character->AttackFinished(ComboResetDelay);
	}
}
