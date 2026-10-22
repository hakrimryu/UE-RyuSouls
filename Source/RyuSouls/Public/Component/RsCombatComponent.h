#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "RsCombatComponent.generated.h"

class ARsWeapon;

// AnimNotifyで確定した抜刀状態をAnimInstanceなどの購読先へ通知する。
DECLARE_MULTICAST_DELEGATE_OneParam(FDelegateOnChangedCombat, bool);

// キャラクターの主武器を保持し、交換時には以前の武器をPickupとしてワールドへ戻す。
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class RYUSOULS_API URsCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// 抜刀状態と連動するアニメーション側が初期化時に登録する変更通知。
	FDelegateOnChangedCombat OnChangedCombat;

protected:
	// 装備側のSetWeapon呼び出しで登録される主武器。UPROPERTYで参照をGCに追跡させる。
	UPROPERTY()
	ARsWeapon* MainWeapon;

	// 主武器を抜刀しているか。装備・解除のAnimNotifyで更新し、拾得時の取り付け先にも使う。
	UPROPERTY(EditAnywhere)
	bool bCombatEnabled = false;

	// 最後に実行した攻撃種別。ComboWindow終了時も同じタグのMontage群で次段を選ぶ。
	UPROPERTY(VisibleAnywhere)
	FGameplayTag LastAttackType;

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
	// 抜刀状態を更新し、AnimInstanceへ戦闘・非戦闘の遷移を通知する。
	void SetCombatEnabled(const bool bEnabled);

	// キャラクターのMontage選択とAnimNotifyの取り付け処理で使う現在の主武器。
	FORCEINLINE ARsWeapon* GetMainWeapon() const { return MainWeapon; }

	// 予約入力からコンボを継続する際の攻撃種別を返す。
	FORCEINLINE FGameplayTag GetLastAttackType() const { return LastAttackType; };
	// 攻撃開始時に選択したタグを、次段のMontage選択用に保持する。
	FORCEINLINE void SetLastAttackType(const FGameplayTag& NewAttackTypeTag) { LastAttackType = NewAttackTypeTag; };
};
