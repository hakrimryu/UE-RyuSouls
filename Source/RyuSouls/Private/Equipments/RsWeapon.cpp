#include "Equipments/RsWeapon.h"

ARsWeapon::ARsWeapon()
{
}

void ARsWeapon::EquipItem()
{
	Super::EquipItem();

	// 所有者が未設定なら、主武器の登録先を取得できないため装備処理を行わない。
	AActor* OwnerActor = GetOwner();
	if (OwnerActor == nullptr)
	{
		return;
	}
	CombatComponent = OwnerActor->GetComponentByClass<URsCombatComponent>();

	if (CombatComponent)
	{
		// 以前の武器を落として自身を主武器に登録し、拾得直後は収納用ソケットへ取り付ける。
		CombatComponent->SetWeapon(this);
		AttachToOwner(UnequipSocketName);
	}
}

void ARsWeapon::UnEquipItem()
{
	Super::UnEquipItem();
}
