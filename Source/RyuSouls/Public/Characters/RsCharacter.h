#pragma once

#include "CoreMinimal.h"
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

private:
	// スプリントとローリングに使うスタミナの残量・消費・回復を管理する。
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	URsAttributeComponent* AttributeComponent;

	// 現在の行動状態と移動入力の許可を管理する。
	URsStateComponent* StateComponent;

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

};
