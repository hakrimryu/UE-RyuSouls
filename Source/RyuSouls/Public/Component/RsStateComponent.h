
#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "RsStateComponent.generated.h"


// 行動状態をGameplayTagで保持し、移動入力の禁止と遅延付きの復帰を管理する。
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class RYUSOULS_API URsStateComponent : public UActorComponent
{
	GENERATED_BODY()

protected:

	// ARsCharacter::Moveが新しい移動入力を受け付けるかどうか。実際の速度や視点操作は制御しない。
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	bool bMoveInputEnabled = true;

	// 現在の行動状態。空のタグは行動状態が設定されていないことを表す。BPから参照できる。
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	FGameplayTag CurrentState;

public:
	URsStateComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:

	// 移動入力をCharacterMovementへ渡してよいかを返す。
	FORCEINLINE bool MovementInputEnabled() const { return bMoveInputEnabled; }

	// falseで即座に入力を禁止し、trueでDuration秒後の入力復帰と状態解除を予約する。
	void ToggleMovementInput(bool bEnable, float Duration = 0.0f);

protected:
	// 遅延処理から名前で呼び出され、移動入力を許可して行動状態を解除する。
	UFUNCTION()
	void MovementInputEnableAction();


public:
	// 行動状態のタグだけを変更する。移動入力の許可は別途切り替える。
	FORCEINLINE void SetState(const FGameplayTag& NewState) { CurrentState = NewState; }
	// 現在の行動状態を参照するためのタグを返す。
	FORCEINLINE FGameplayTag GetCurrentState() const { return CurrentState; }

	// 行動状態を空のタグに戻す。移動入力の許可は変更しない。
	void ClearState();

	// 候補のいずれかと現在のタグが完全一致するかを判定する。親タグへの一致は含めない。
	bool IsCurrentStateEqualToAny(const FGameplayTagContainer& TagsToCheck) const;
};
