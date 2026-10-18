#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Equipments/RsEquipment.h"
#include "RsWeapon.generated.h"

class URsWeaponCollisionComponent;
class URsMontageActionData;
class URsCombatComponent;
struct FGameplayTag;

UCLASS()
class RYUSOULS_API ARsWeapon : public ARsEquipment
{
	GENERATED_BODY()

protected:
	// 抜刀時の取り付け先。手側のソケット名を武器BPで指定し、AnimNotifyと抜刀中の拾得で使う。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment | Socket")
	FName EquipSocketName;

	// 納刀時の取り付け先。収納側のソケット名を武器BPで指定し、AnimNotifyと納刀中の拾得で使う。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment | Socket")
	FName UnequipSocketName;

	// 装備・解除のGameplayTagとMontage群を対応付けるDataAsset。各武器BPで割り当てる。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment | Animation")
	URsMontageActionData* MontageActionData;

protected:
	// 武器ソケット間の命中判定を管理し、Notifyで有効区間を切り替える。
	UPROPERTY(VisibleAnywhere)
	URsWeaponCollisionComponent* WeaponCollision;

protected:
	// EquipItemでOwnerから取得し、この武器を主武器として登録するための参照。
	UPROPERTY()
	URsCombatComponent* CombatComponent;

protected:
	// 攻撃タグごとの1回分のスタミナ消費量。武器BPで既定値を調整できる。
	UPROPERTY(EditAnywhere)
	TMap<FGameplayTag, float> StaminaCostMap;

	// 攻撃種別の倍率を掛ける前の基礎ダメージ。武器BPで調整できる。
	UPROPERTY(EditAnywhere)
	float BaseDamage = 15.0f;

	// 攻撃タグごとのダメージ倍率。未登録のタグは基礎ダメージをそのまま使う。
	UPROPERTY(EditAnywhere)
	TMap<FGameplayTag, float> DamageMultiplierMap;

public:
	ARsWeapon();

public:
	// Ownerの主武器に登録し、抜刀状態に応じたソケットへ取り付ける。呼び出し前にOwnerを設定する。
	virtual void EquipItem() override;

	// 指定タグのMontage群からIndex番目を返す。通常の装備切り替えでは既定の0番目を使う。
	UAnimMontage* GetMontageForTag(const FGameplayTag& Tag, const int32 Index = 0) const;

	// 指定攻撃の消費量を返す。未登録のタグは消費量0として扱う。
	float GetStaminaCost(const FGameplayTag& InTag) const;

	// CombatComponentの直前の攻撃タグから倍率を選び、今回のダメージ量を返す。
	float GetAttackDamage() const;

	// AnimNotifyが抜刀・納刀の取り付け先を選ぶためのソケット名。
	FORCEINLINE FName GetEquipSocketName() const { return EquipSocketName; }
	FORCEINLINE FName GetUnequipSocketName() const { return UnequipSocketName; }
	// Weapon CollisionのAnimNotifyStateがトレースの開始・停止に使う参照。
	FORCEINLINE URsWeaponCollisionComponent* GetCollision() const { return WeaponCollision; };

public:
	// 命中イベントを受け取り、相手へ武器を加害元とするPointDamageを渡す。
	void OnHitActor(const FHitResult& Hit);
};
