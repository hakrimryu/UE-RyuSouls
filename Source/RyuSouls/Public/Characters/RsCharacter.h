#pragma once

#include "CoreMinimal.h"
#include "Component/RsCombatComponent.h"
#include "GameFramework/Character.h"
#include "RsCharacter.generated.h"

struct FInputActionValue;
class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class URsAttributeComponent;
class URsPlayerHUDWidget;
class URsStateComponent;

UCLASS()
class RYUSOULS_API ARsCharacter : public ACharacter
{
	GENERATED_BODY()

private:
	// カメラまでの距離と向きを管理する親コンポーネント。
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	// 実際に画面を映すカメラ。CameraBoomの子として追従する。
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowBoom;

	// EditAnywhereはエディタで値を指定できる指定。BP_RsCharacterで使用する入力アセットを割り当てる。
	// Mapping Contextは「どのキーをどのActionに変換するか」の設定集。
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputMappingContext* DefaultInputMappingContext;

	// 移動という操作を表すAction。このコードではAxis2Dの値を「X=左右、Y=前後」として読む。
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* MoveAction;

	// 視点操作のAction。Axis2Dの値を「X=左右回転、Y=上下回転」として読む。
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* LookAction;

	// BPで割り当てる共通Action。Triggeredでスプリント、Completedで終了、Canceledでローリングを処理する。
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* SprintRollingAction;

	// BP_RsCharacterでIA_Interactionを割り当て、Eキーの入力開始時に周囲の対象を調べる。
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* InteractAction;

	// BP_RsCharacterでIA_ToggleCombatを割り当て、Rキーで抜刀・納刀を切り替える。
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* ToggleCombatAction;

	// IA_Attackの短押しで通常攻撃、0.7秒の長押しで特殊攻撃を要求する。
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* AttackAction;

	// IA_Shiftとの同時入力を条件とするIA_HeavyAttackをBPで割り当てる。
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* HeavyAttackAction;

private:
	// スプリントとローリングに使うスタミナの残量・消費・回復を管理する。
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	URsAttributeComponent* AttributeComponent;

	// 現在の行動状態と移動入力の許可を管理する。
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	URsStateComponent* StateComponent;

	// このキャラクターが装備する主武器の参照を保持する。
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	URsCombatComponent* CombatComponent;

protected:
	// BP_RsCharacterで指定するHUDのクラス。URsPlayerHUDWidgetを継承するWBP_PlayerHUDを割り当てる。
	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UUserWidget> PlayerHUDWidgetClass;

	// 開始時に生成して画面へ追加するHUD。UPROPERTYで参照を保持してGCの対象から保護する。
	UPROPERTY()
	URsPlayerHUDWidget* PlayerHUDWidget;

protected:
	// スプリント時にCharacterMovementへ設定する速度上限（cm/秒）。BPで調整できる。
	UPROPERTY(EditAnywhere, Category = "Sprinting")
	float SprintingSpeed = 750.0f;

	// スプリント終了時に戻す通常の速度上限（cm/秒）。BPで調整できる。
	UPROPERTY(EditAnywhere, Category = "Sprinting")
	float NormalSpeed = 500.0f;

	// スプリント処理で更新し、通常入力から走行攻撃を選ぶために使う。
	UPROPERTY(VisibleAnywhere, Category = "Sprinting")
	bool bSprinting = false;

protected:
	// 攻撃終了後もリセット待ちの間は保持し、次の入力でコンボを継続する。
	bool bComboSequenceRunning = false;

	// ComboWindowのNotify区間内だけ追加入力を受け付ける。
	bool bCanComboInput = false;

	// 同じ攻撃タグのMontage配列を選ぶ0始まりのIndex。
	int32 ComboCounter = 0;

	// 入力受付区間中の入力を保持し、区間終了時に次の攻撃へ進める。
	bool bSavedComboInput = false;

	// 攻撃終了後のコンボ継続期限を管理し、次の攻撃開始時に取り消す。
	FTimerHandle ComboResetTimerHandle;

protected:
	// BPで割り当てるローリング用Montage。ResetMovementInput通知で移動入力を復帰させる。
	UPROPERTY(EditAnywhere, Category = "Montages")
	UAnimMontage* RollMontage;

public:
	ARsCharacter();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	// このキャラクターを操作するControllerが変わったときに、ローカルプレイヤーの入力設定を登録する。
	virtual void NotifyControllerChanged() override;

	// 入力を受け取る準備時に、各Actionと呼び出す関数を結び付ける。
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

public:
	// AnimInstanceなどが行動状態と移動入力の許可を操作するための参照を返す。
	FORCEINLINE URsStateComponent* GetStateComponent() const { return StateComponent; }

protected:

	// 水平方向に移動し、移動入力による加速度もあるかを調べる。
	bool IsMoving() const;
	// 攻撃・ローリング・装備切り替えの状態ではないことを確認する。
	bool CanToggleCombat() const;

	// 走行攻撃を選択するため、スプリント入力処理で保持した状態を返す。
	FORCEINLINE bool IsSprinting() const { return bSprinting; }

	// 2Dの移動入力を、操作方向を基準にした3D空間の移動入力へ変換する。
	void Move(const FInputActionValue& Values);
	// 2Dの視点入力を、Controllerの左右・上下の回転入力へ変換する。
	void Look(const FInputActionValue& Values);

	// 移動とスタミナの条件を満たす間、速度上限を上げてスタミナを消費する。
	void Sprinting();

	// 通常の速度上限へ戻し、遅延付きのスタミナ回復を有効にする。
	void StopSprint();

	// スタミナが足りる場合に移動入力を制限し、ローリングの再生と状態更新を行う。
	void Rolling();

	// 専用Object Channelを持つ周囲100cm以内の対象を検出し、インターフェース経由で操作を渡す。
	void Interact();

	// 主武器の抜刀状態に応じたMontageを再生し、ResetState通知まで次の切り替えを抑止する。
	void ToggleCombat();

	// 納刀中の攻撃入力で抜刀を要求する。攻撃自体の予約は行わない。
	void AutoToggleCombat();

	// 短押し入力から通常・走行攻撃を選び、実行条件を満たせばコンボ処理へ渡す。
	void Attack();

	// 長押し成立時に特殊攻撃を要求する。
	void SpecialAttack();

	// 強攻撃を要求する。納刀中なら先に抜刀を開始する。
	void HeavyAttack();

protected:
	// スプリント状態に応じて通常攻撃か走行攻撃のタグを返す。
	FGameplayTag GetAttackPerform() const;

	// 主武器・抜刀状態・行動状態・指定攻撃のスタミナ残量を確認する。
	bool CanPerformAttack(const FGameplayTag& AttackTypeTag) const;
	// 攻撃状態と移動制限を設定し、コンボ段数に対応するMontage再生とスタミナ消費を行う。
	void DoAttack(const FGameplayTag& AttackTypeTag);
	// 新規攻撃・終了後の継続入力・Notify区間内の先行入力を振り分ける。
	void ExecuteComboAttack(const FGameplayTag& AttackTypeTag);
	// コンボの段数と入力受付・予約・継続状態を初期化する。
	void ResetCombo();

public:
	// ComboWindowのNotifyBeginから呼び出し、次の攻撃の入力受付を開始する。
	void EnableComboWindow();
	// ComboWindowのNotifyEndから呼び出し、予約入力があれば直前の攻撃種別で次段を再生する。
	void DisableComboWindow();
	// AttackFinished通知から移動復帰を要求し、ComboResetDelay秒後にコンボをリセットする。
	void AttackFinished(const float ComboResetDelay);
};
