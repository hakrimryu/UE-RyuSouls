#include "Equipments/RsWeapon.h"

#include "RsGameplayTags.h"
#include "Animations/RsAnimInstance.h"
#include "Characters/RsCharacter.h"
#include "Component/RsCombatComponent.h"
#include "Component/RsWeaponCollisionComponent.h"
#include "Data/RsMontageActionData.h"
#include "Kismet/GameplayStatics.h"

ARsWeapon::ARsWeapon()
{
	// 命中判定を武器ごとに保持し、検出結果をこの武器のダメージ処理へ接続する。
	WeaponCollision = CreateDefaultSubobject<URsWeaponCollisionComponent>("MainCollision");
	WeaponCollision->OnHitActor.AddUObject(this, &ThisClass::OnHitActor);

	// 複数の攻撃判定を持つ武器向けに、主判定とは独立して切り替えられる判定を用意する。
	SecondWeaponCollision = CreateDefaultSubobject<URsWeaponCollisionComponent>("SecondCollision");
	SecondWeaponCollision->OnHitActor.AddUObject(this, &ThisClass::OnHitActor);

	// 攻撃1回あたりの消費量。実行可否の確認と再生開始時の消費に同じ値を使う。
	StaminaCostMap.Add(RsGameplayTags::Character_Attack_Light, 7.0f);
	StaminaCostMap.Add(RsGameplayTags::Character_Attack_Running, 12.0f);
	StaminaCostMap.Add(RsGameplayTags::Character_Attack_Special, 15.0f);
	StaminaCostMap.Add(RsGameplayTags::Character_Attack_Heavy, 20.0f);

	// 通常攻撃は基礎値、強攻撃・走行攻撃は1.8倍、特殊攻撃は2.1倍を既定値にする。
	DamageMultiplierMap.Add(RsGameplayTags::Character_Attack_Heavy, 1.8f);
	DamageMultiplierMap.Add(RsGameplayTags::Character_Attack_Running, 1.8f);
	DamageMultiplierMap.Add(RsGameplayTags::Character_Attack_Special, 2.1f);
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

		// 装備の表示メッシュをソケット位置の取得元として登録する。
		WeaponCollision->SetWeaponMesh(Mesh);

		// 武器BPの種別をAnimInstanceへ渡し、装備中の戦闘用BlendSpaceを選択させる。
		if (ARsCharacter* OwnerCharacter = Cast<ARsCharacter>(GetOwner()))
		{
			if (URsAnimInstance* Anim = Cast<URsAnimInstance>(OwnerCharacter->GetMesh()->GetAnimInstance()))
			{
				Anim->UpdateCombatMode(CombatType);
			}
		}

		// 武器自身に加え、所有キャラクターにも自分の攻撃が命中しないよう検索から除外する。
		WeaponCollision->AddIgnoredActor(GetOwner());
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

float ARsWeapon::GetAttackDamage() const
{
	if (const AActor* OwnerActor = GetOwner())
	{
		const FGameplayTag LastAttackType = CombatComponent->GetLastAttackType();

		// 実行済みの攻撃タグに倍率がある場合だけ適用し、それ以外は基礎値を返す。
		if (DamageMultiplierMap.Contains(LastAttackType))
		{
			const float Multiplier = DamageMultiplierMap[LastAttackType];
			return BaseDamage * Multiplier;
		}
	}

	return BaseDamage;
}

void ARsWeapon::OnHitActor(const FHitResult& Hit)
{
	AActor* TargetActor = Hit.GetActor();

	// PointDamageの方向には、衝突面の法線ではなく攻撃者の正面方向を渡す。
	FVector DamageDirection = GetOwner()->GetActorForwardVector();

	// 直前の攻撃種別に対応する武器の基礎ダメージと倍率から算出する。
	float AttackDamage = GetAttackDamage();

	// Instigatorは攻撃者のController、DamageCauserはこの武器として命中結果を通知する。
	UGameplayStatics::ApplyPointDamage(
		TargetActor,
		AttackDamage,
		DamageDirection,
		Hit,
		GetOwner()->GetInstigatorController(),
		this,
		nullptr);
}


void ARsWeapon::ActiveCollision(EWeaponCollisionType InCollisionType)
{
	switch (InCollisionType)
	{
	case EWeaponCollisionType::MainCollision:
		WeaponCollision->TurnOnCollision();
		break;

	case EWeaponCollisionType::SecondCollision:
		SecondWeaponCollision->TurnOnCollision();
		break;
	}
}

void ARsWeapon::DeactivateCollision(EWeaponCollisionType InCollisionType)
{
	switch (InCollisionType)
	{
	case EWeaponCollisionType::MainCollision:
		WeaponCollision->TurnOffCollision();
		break;

	case EWeaponCollisionType::SecondCollision:
		SecondWeaponCollision->TurnOffCollision();
		break;
	}
}
