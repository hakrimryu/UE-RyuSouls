#include "Equipments/RsFistWeapon.h"

#include "Animations/RsAnimInstance.h"
#include "Characters/RsCharacter.h"
#include "Component/RsCombatComponent.h"
#include "Component/RsWeaponCollisionComponent.h"

ARsFistWeapon::ARsFistWeapon()
{
}

void ARsFistWeapon::EquipItem()
{
	// 通常武器のMeshを持たないため、OwnerのCombatComponentへ直接登録する。
	CombatComponent = GetOwner()->GetComponentByClass<URsCombatComponent>();

	if (CombatComponent)
	{
		CombatComponent->SetWeapon(this);

		if (ARsCharacter* OwnerCharacter = Cast<ARsCharacter>(GetOwner()))
		{
			// 左右の手に設定したソケットを使うため、両方の判定へキャラクターのMeshを登録する。
			WeaponCollision->SetWeaponMesh(OwnerCharacter->GetMesh());
			SecondWeaponCollision->SetWeaponMesh(OwnerCharacter->GetMesh());

			// 素手は抜刀Montageを使わず、生成時から戦闘状態にする。
			CombatComponent->SetCombatEnabled(true);

			// AnimInstanceへ素手用の戦闘アニメーション種別を反映する。
			if (URsAnimInstance* Anim = Cast<URsAnimInstance>(OwnerCharacter->GetMesh()->GetAnimInstance()))
			{
				Anim->UpdateCombatMode(CombatType);
			}

			// 左右どちらの攻撃判定でも、所有キャラクター自身を命中対象から除外する。
			WeaponCollision->AddIgnoredActor(OwnerCharacter);
			SecondWeaponCollision->AddIgnoredActor(OwnerCharacter);
		}
	}
	
}
