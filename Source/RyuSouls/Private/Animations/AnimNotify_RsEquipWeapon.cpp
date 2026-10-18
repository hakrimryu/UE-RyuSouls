#include "Animations/AnimNotify_RsEquipWeapon.h"

#include "RsGameplayTags.h"
#include "Component/RsCombatComponent.h"
#include "Equipments/RsWeapon.h"

UAnimNotify_RsEquipWeapon::UAnimNotify_RsEquipWeapon(const FObjectInitializer& ObjectInitializer)
{
}

void UAnimNotify_RsEquipWeapon::Notify(USkeletalMeshComponent* MeshComponent, UAnimSequenceBase* Animation,
                                       const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComponent, Animation, EventReference);

	// アニメーションプレビューなど、主武器を持つ所有者がいない場合は取り付け処理を行わない。
	if (const AActor* Owner = MeshComponent->GetOwner())
	{
		if (URsCombatComponent* CombatComponent = Owner->GetComponentByClass<URsCombatComponent>())
		{
			if (ARsWeapon* MainWeapon = CombatComponent->GetMainWeapon())
			{
				bool bCombatEnabled = CombatComponent->IsCombatEnabled();
				FName WeaponSocketName;

				// 同じ通知クラスを両Montageで使い、通知ごとのタグで手側・収納側を選ぶ。
				if (MontageActionTag == RsGameplayTags::Character_Action_Equip)
				{
					bCombatEnabled = true;
					WeaponSocketName = MainWeapon->GetEquipSocketName();
				}
				else if (MontageActionTag == RsGameplayTags::Character_Action_Unequip)
				{
					bCombatEnabled = false;
					WeaponSocketName = MainWeapon->GetUnequipSocketName();
				}

				// 通知の時点で抜刀状態を更新してから、同じタグに対応するソケットへ武器を移す。
				CombatComponent->SetCombatEnabled(bCombatEnabled);
				MainWeapon->AttachToOwner(WeaponSocketName);
			}
		}
	}
}
