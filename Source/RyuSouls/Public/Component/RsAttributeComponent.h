#pragma once

#include "CoreMinimal.h"
#include "RsDefine.h"
#include "Components/ActorComponent.h"
#include "RsAttributeComponent.generated.h"

// HUDへ属性種別と残量比率を渡すC++用の通知。複数の購読先へ同じ変更を届ける。
DECLARE_MULTICAST_DELEGATE_TwoParams(FDelegateOnAttributeChanged, ERsAttributeType, float);

// スタミナの残量・消費・タイマーによる自動回復を管理する。
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class RYUSOULS_API URsAttributeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// 消費・回復後の比率や、HUD初期表示用の現在値を通知する。
	FDelegateOnAttributeChanged OnAttributeChanged;

protected:
	// 現在のスタミナ残量。消費と回復で更新する。
	UPROPERTY(EditAnywhere, Category="Stamina")
	float BaseStamina = 100.0f;

	// 消費・回復時に残量を制限する上限値。
	UPROPERTY(EditAnywhere, Category="Stamina")
	float MaxStamina = 100.0f;

	// 回復タイマー1回あたりの回復量。間隔は0.1秒なので、既定値では秒あたり2.0回復する。
	UPROPERTY(EditAnywhere, Category="Stamina")
	float StaminaRegenRate = 0.2f;

	// 回復開始待ちと定期回復を同じタイマーで管理するためのハンドル。
	FTimerHandle StaminaRegenTimerHandle;

public:
	URsAttributeComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	// キャラクター側の残量判定や確認用表示で使う読み取り専用の取得関数。
	FORCEINLINE float GetBaseStamina() const { return BaseStamina; }
	FORCEINLINE float GetMaxStamina() const { return MaxStamina; }

	// ゲージ用に現在値 / 最大値を返す。MaxStaminaが正の値であることを前提とする。
	FORCEINLINE float GetStaminaRatio() const { return BaseStamina / MaxStamina; }

	// 残量が指定量以上かを調べる。判定のみでスタミナは消費しない。
	bool CheckHasEnoughStamina(float StaminaCost) const;

	// 指定量を消費し、残量を0から最大値の範囲に制限する。
	void DecreaseStamina(float StaminaCost);

	// 回復の有効・無効を切り替える。StartDelayは初回回復までの秒数で、有効なタイマーは再開始しない。
	void ToggleStaminaRegeneration(bool bEnable, float StartDelay = 2.0f);

	// 指定した属性の比率を購読先へ通知する。値を変更せず、HUD生成直後の初期表示にも使う。
	void BroadcastAttributeChanged(ERsAttributeType InAttributeType) const;

private:
	// 回復タイマーから呼ばれ、1回分を回復する。最大値に達したらタイマーを停止する。
	void RegenerateStaminaHandler();

};
