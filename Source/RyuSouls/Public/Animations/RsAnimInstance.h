
#pragma once

#include "CoreMinimal.h"
#include "RsDefine.h"
#include "Animation/AnimInstance.h"
#include "RsAnimInstance.generated.h"

class UCharacterMovementComponent;

// キャラクターの移動状態を取得し、ABP_RsCharacterのブレンドと状態遷移に渡す。
UCLASS()
class RYUSOULS_API URsAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

protected:
	// このアニメーションを使用するキャラクター。初期化時に所有Actorから取得する。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
	ACharacter* Character;

	// 実際の速度・加速度・落下状態を読み取る移動コンポーネント。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
	UCharacterMovementComponent* MovementComponent;

protected:
	// 上下方向を含むワールド空間の移動速度。標準のUE単位ではcm/秒。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Data")
	FVector Velocity;

	// Z成分を除いた水平速度。移動BlendSpaceのSpeed軸へ渡して待機と移動をブレンドする。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Data")
	float GroundSpeed;

	// 水平速度と加速度から求める移動判定。ABPの待機・移動の切り替えと着地からの復帰に使う。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Data")
	bool bShouldMove;

	// CharacterMovementが落下モードかどうか。ABPの落下・着地への遷移に使う。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Data")
	bool bIsFalling;

	// Actorの正面を基準にした水平移動方向（度）。方向別の戦闘移動BlendSpaceへ渡す。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Data")
	float Direction;

protected:
	// AnimNotifyで更新された抜刀状態。AnimGraphの非戦闘・戦闘遷移条件に使う。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Data")
	bool bCombatEnabled = false;

	// 現在の武器種別。AnimGraphで剣盾・両手武器などの戦闘アニメーションを選ぶ。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Data")
	ECombatType CombatType = ECombatType::None;

public:
	URsAnimInstance();

	// アニメーションの初期化時に、移動状態を取得するための参照を準備する。
	virtual void NativeInitializeAnimation() override;
	// アニメーションの更新ごとに、ABPで使用する移動データを更新する。
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

public:
	// アニメーションのResetMovementInput通知を受け、移動入力の復帰と行動状態の解除を要求する。
	UFUNCTION()
	void AnimNotify_ResetMovementInput();

	// MontageのResetState通知で行動状態だけを解除する。名前で呼ばれるためUFUNCTION登録が必要。
	UFUNCTION()
	void AnimNotify_ResetState();

public:
	// 装備した武器の種別を受け取り、AnimGraphが使用する戦闘アニメーションを切り替える。
	void UpdateCombatMode(const ECombatType InCombatType);

protected:
	// CombatComponentの通知を受け、AnimGraphへ公開する抜刀状態を同期する。
	void OnChangedCombat(const bool bInCombatEnabled);
};
