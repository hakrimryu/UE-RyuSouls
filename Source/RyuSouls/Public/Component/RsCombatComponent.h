#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RsCombatComponent.generated.h"

class ARsWeapon;

// キャラクターの主武器を保持し、交換時には以前の武器をPickupとしてワールドへ戻す。
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class RYUSOULS_API URsCombatComponent : public UActorComponent
{
	GENERATED_BODY()

protected:
	// 装備側のSetWeapon呼び出しで登録される主武器。UPROPERTYで参照をGCに追跡させる。
	UPROPERTY()
	ARsWeapon* MainWeapon;

	// 主武器を抜刀しているか。装備・解除のAnimNotifyで更新し、拾得時の取り付け先にも使う。
	UPROPERTY(EditAnywhere)
	bool bCombatEnabled = false;

public:
	URsCombatComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	// 以前の主武器があれば同じ装備クラスのPickupへ置き換え、新しい参照を登録する。取り付けは武器側が行う。
	void SetWeapon(ARsWeapon* NewWeapon);

public:
	// Montageの選択と武器交換時に参照する抜刀状態。
	FORCEINLINE bool IsCombatEnabled() const { return bCombatEnabled; }
	// AnimNotifyの切り替え時点で状態を更新する。ソケットへの取り付けは通知側が続けて行う。
	FORCEINLINE void SetCombatEnabled(bool bEnabled) { bCombatEnabled = bEnabled; }

	// キャラクターのMontage選択とAnimNotifyの取り付け処理で使う現在の主武器。
	FORCEINLINE ARsWeapon* GetMainWeapon() const { return MainWeapon; }
};
