#include "Animations/AnimNotifyState_RsWeaponHit.h"

#include "Component/RsCombatComponent.h"
#include "Component/RsWeaponCollisionComponent.h"
#include "Equipments/RsWeapon.h"

UAnimNotifyState_RsWeaponHit::UAnimNotifyState_RsWeaponHit(const FObjectInitializer& ObjectInitializer)
{
}

void UAnimNotifyState_RsWeaponHit::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	// プレビュー用Actorなどを除外し、CombatComponentに登録された有効な主武器だけを操作する。
	if (const AActor* OwnerActor = MeshComp->GetOwner())
	{
		if (const URsCombatComponent* CombatComponent = OwnerActor->GetComponentByClass<URsCombatComponent>())
		{
			const ARsWeapon* Weapon = CombatComponent->GetMainWeapon();
			if (::IsValid(Weapon))
			{
				Weapon->GetCollision()->TurnOnCollision();
			}
		}
	}
}

void UAnimNotifyState_RsWeaponHit::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	// 終了時点の主武器を取得して判定を止め、攻撃区間外の命中通知を抑止する。
	if (const AActor* OwnerActor = MeshComp->GetOwner())
	{
		if (const URsCombatComponent* CombatComponent = OwnerActor->GetComponentByClass<URsCombatComponent>())
		{
			const ARsWeapon* Weapon = CombatComponent->GetMainWeapon();
			if (::IsValid(Weapon))
			{
				Weapon->GetCollision()->TurnOffCollision();
			}
		}
	}
}
