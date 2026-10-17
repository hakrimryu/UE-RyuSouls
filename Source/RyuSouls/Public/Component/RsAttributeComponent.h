#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RsAttributeComponent.generated.h"


// スタミナの残量・消費・タイマーによる自動回復を管理する。
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class RYUSOULS_API URsAttributeComponent : public UActorComponent
{
	GENERATED_BODY()
	
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
	
	// 残量が指定量以上かを調べる。判定のみでスタミナは消費しない。
	bool CheckHasEnoughStamina(float StaminaCost) const;
	
	// 指定量を消費し、残量を0から最大値の範囲に制限する。
	void DecreaseStamina(float StaminaCost);
	
	// 回復の有効・無効を切り替える。StartDelayは初回回復までの秒数で、有効なタイマーは再開始しない。
	void ToggleStaminaRegeneration(bool bEnable, float StartDelay = 2.0f);
	
private:
	// 回復タイマーから呼ばれ、1回分を回復する。最大値に達したらタイマーを停止する。
	void RegenerateStaminaHandler();
		
};
