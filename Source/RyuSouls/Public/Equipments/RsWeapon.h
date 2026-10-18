#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Equipments/RsEquipment.h"
#include "RsWeapon.generated.h"

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
	// EquipItemでOwnerから取得し、この武器を主武器として登録するための参照。
	UPROPERTY()
	URsCombatComponent* CombatComponent;

protected:
	// 攻撃タグごとの1回分のスタミナ消費量。武器BPで既定値を調整できる。
	UPROPERTY(EditAnywhere)
	TMap<FGameplayTag, float> StaminaCostMap;

public:
	ARsWeapon();

public:
	// Ownerの主武器に登録し、抜刀状態に応じたソケットへ取り付ける。呼び出し前にOwnerを設定する。
	virtual void EquipItem() override;

	// 指定タグのMontage群からIndex番目を返す。通常の装備切り替えでは既定の0番目を使う。
	UAnimMontage* GetMontageForTag(const FGameplayTag& Tag, const int32 Index = 0) const;

	// 指定攻撃の消費量を返す。未登録のタグは消費量0として扱う。
	float GetStaminaCost(const FGameplayTag& InTag) const;

	// AnimNotifyが抜刀・納刀の取り付け先を選ぶためのソケット名。
	FORCEINLINE FName GetEquipSocketName() const { return EquipSocketName; }
	FORCEINLINE FName GetUnequipSocketName() const { return UnequipSocketName; }
};
