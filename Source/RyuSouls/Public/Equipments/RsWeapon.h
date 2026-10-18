#pragma once

#include "CoreMinimal.h"
#include "Component/RsCombatComponent.h"
#include "Equipments/RsEquipment.h"
#include "RsWeapon.generated.h"

UCLASS()
class RYUSOULS_API ARsWeapon : public ARsEquipment
{
	GENERATED_BODY()

protected:
	// 抜刀時の取り付け先として用意した名前。現在の拾得処理では使わず、手側のソケット名をBPで指定する。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment | Socket")
	FName EquipSocketName;

	// 拾得直後の収納先。現在はEquipItemでこのソケットへ取り付け、UnEquipItemはまだ切り替えを行わない。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment | Socket")
	FName UnequipSocketName;

protected:
	// EquipItemでOwnerから取得し、この武器を主武器として登録するための参照。
	UPROPERTY()
	URsCombatComponent* CombatComponent;


public:
	ARsWeapon();

public:
	// OwnerのCombatComponentに自身を登録し、収納用ソケットへ取り付ける。呼び出し前にOwnerを設定する。
	virtual void EquipItem() override;

	// 装備解除処理の入口。現在は基底処理のみで、参照の解除やソケットの切り替えは未実装。
	virtual void UnEquipItem() override;
};
