#include "Animations/AnimNotifyState_RsWeaponHit.h"

#include "Component/RsCombatComponent.h"
#include "Component/RsWeaponCollisionComponent.h"
#include "Equipments/RsWeapon.h"
#include "Interface/RsCombatInterface.h"

UAnimNotifyState_RsWeaponHit::UAnimNotifyState_RsWeaponHit(const FObjectInitializer& ObjectInitializer)
{
}

void UAnimNotifyState_RsWeaponHit::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	// 装備形態ごとの判定選択はOwnerへ委ね、Notifyを特定の武器実装から分離する。
	if (AActor* OwnerActor = MeshComp->GetOwner())
	{
		if (IRsCombatInterface* CombatInterface = Cast<IRsCombatInterface>(OwnerActor))
		{
			CombatInterface->ActivateWeaponCollision(CollisionType);
		}
	}
}

void UAnimNotifyState_RsWeaponHit::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	// 開始時と同じ判定種別をOwnerへ渡し、攻撃区間外のトレースを停止する。
	if (AActor* OwnerActor = MeshComp->GetOwner())
	{
		if (IRsCombatInterface* CombatInterface = Cast<IRsCombatInterface>(OwnerActor))
		{
			CombatInterface->DeactivateWeaponCollision(CollisionType);
		}
	}
}
