#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "RsCharacter.generated.h"

struct FInputActionValue;
class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;

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
	UPROPERTY(EditAnywhere)
	UInputMappingContext* DefaultInputMappingContext;
	
	// 移動という操作を表すAction。このコードではAxis2Dの値を「X=左右、Y=前後」として読む。
	UPROPERTY(EditAnywhere)
	UInputAction* MoveAction;
	
	// 視点操作のAction。Axis2Dの値を「X=左右回転、Y=上下回転」として読む。
	UPROPERTY(EditAnywhere)
	UInputAction* LookAction;
	
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

protected:
	// 2Dの移動入力を、操作方向を基準にした3D空間の移動入力へ変換する。
	void Move(const FInputActionValue& Values);
	// 2Dの視点入力を、Controllerの左右・上下の回転入力へ変換する。
	void Look(const FInputActionValue& Values);
};
