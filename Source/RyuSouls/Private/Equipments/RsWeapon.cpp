#include "Equipments/RsWeapon.h"

#include "RsGameplayTags.h"
#include "Component/RsCombatComponent.h"
#include "Data/RsMontageActionData.h"

ARsWeapon::ARsWeapon()
{
	// 攻撃1回あたりの消費量。実行可否の確認と再生開始時の消費に同じ値を使う。
	StaminaCostMap.Add(RsGameplayTags::Character_Attack_Light, 7.f);
	StaminaCostMap.Add(RsGameplayTags::Character_Attack_Running, 12.f);
	StaminaCostMap.Add(RsGameplayTags::Character_Attack_Special, 15.f);
	StaminaCostMap.Add(RsGameplayTags::Character_Attack_Heavy, 20.f);
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
		// 以前の武器を落として自身を主武器に登録する。抜刀中の交換では新しい武器も手に持たせる。
		CombatComponent->SetWeapon(this);

		const FName AttachSocket = CombatComponent->IsCombatEnabled() ? EquipSocketName : UnequipSocketName;
		AttachToOwner(AttachSocket);
	}
}

UAnimMontage* ARsWeapon::GetMontageForTag(const FGameplayTag& Tag, const int32 Index) const
{
	// 武器BPに割り当てたDataAssetへ検索を委ね、キャラクター側に個別のMontage参照を持たせない。
	return MontageActionData->GetMontageForTag(Tag, Index);
}

float ARsWeapon::GetStaminaCost(const FGameplayTag& InTag) const
{
	if (StaminaCostMap.Contains(InTag))
	{
		return StaminaCostMap[InTag];
	}
	return 0.f;
}
