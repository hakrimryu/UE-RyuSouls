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
		// 登録先がある場合だけ、主武器として保持させて装備用ソケットへ取り付ける。
		CombatComponent->SetWeapon(this);
		AttachToOwner(EquipSocketName);
	}
}

void ARsWeapon::UnEquipItem()
{
	Super::UnEquipItem();
}
