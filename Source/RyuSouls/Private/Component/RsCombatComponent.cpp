#include "Component/RsCombatComponent.h"

#include "Characters/RsCharacter.h"
#include "Equipments/RsWeapon.h"
#include "Items/RsPickupItem.h"

URsCombatComponent::URsCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void URsCombatComponent::BeginPlay()
{
	Super::BeginPlay();
}

void URsCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void URsCombatComponent::SetWeapon(ARsWeapon* NewWeapon)
{
	// 以前の主武器が有効なら、所有キャラクターの位置に落としてから新しい武器へ交換する。
	if (::IsValid(MainWeapon))
	{
		if (ARsCharacter* OwnerCharacter = Cast<ARsCharacter>(GetOwner()))
		{
			// BP_Pickupを使わずC++クラスから生成する。装備クラスを設定するまでConstructionの実行を遅らせる。
			ARsPickupItem* PickupItem = GetWorld()->SpawnActorDeferred<ARsPickupItem>(
				ARsPickupItem::StaticClass(),
				OwnerCharacter->GetActorTransform(),
				nullptr,
				nullptr,
				ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);

			// 生成に失敗した場合は参照先へアクセスせず、以前の主武器を保持したまま戻る。
			if (!ensure(PickupItem))
			{
				return;
			}

			// 実際の武器のBPクラスを渡し、FinishSpawning時にその既定メッシュと物理設定をPickupへ反映する。
			PickupItem->SetEquipmentClass(MainWeapon->GetClass());
			PickupItem->FinishSpawning(GetOwner()->GetActorTransform());

			// ワールド側のPickupを構築した後、キャラクターに付いていた古い装備Actorを破棄する。
			MainWeapon->Destroy();
		}
	}

	// 武器から渡された参照を保持し、取り付け処理は装備側に任せる。
	MainWeapon = NewWeapon;
}

void URsCombatComponent::SetCombatEnabled(const bool bEnabled)
{
	bCombatEnabled = bEnabled;

	// AnimNotifyで確定した状態を、その値を使うAnimInstanceへ即座に反映する。
	if (OnChangedCombat.IsBound())
	{
		OnChangedCombat.Broadcast(bCombatEnabled);
	}
}
