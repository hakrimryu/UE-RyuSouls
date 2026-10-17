#include "Characters/RsCharacter.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"

ARsCharacter::ARsCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	
	// キャラクターのルート → スプリングアーム → カメラ、という親子関係を作る。
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	// 標準のUE単位では400cm。FRotatorの引数はPitch（上下）、Yaw（左右）、Roll（傾き）の順。
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->SetRelativeRotation(FRotator(-30.0f, 0.0f, 0.0f));
	
	FollowBoom = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowBoom"));
	FollowBoom->SetupAttachment(CameraBoom);

}

void ARsCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

void ARsCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ARsCharacter::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();

	// Controllerはこのキャラクターの操作主体。CastはPlayerControllerかを調べ、違えばnullptrになる。
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		// そのローカルプレイヤーの入力管理機能を取得する。ローカルプレイヤーがなければ登録しない。
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			// BPで指定したキー割り当てを有効にする。0は優先度で、入力値やプレイヤー番号ではない。
			Subsystem->AddMappingContext(DefaultInputMappingContext, 0);
		}
	}
}

void ARsCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Actionと呼び出す処理を結び付ける。thisが呼び出し先、&クラス名::関数名が処理。
		// TriggeredはActionの発火条件を満たしたときの通知。発火頻度はアセットのTrigger設定による。
		// ThisClassはこのUCLASSを指す別名なので、ここではARsCharacterと同じクラスを指す。
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ThisClass::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ARsCharacter::Look);
	}

}

void ARsCharacter::Move(const FInputActionValue& Values)
{
	// Actionの値を2Dベクトルとして取り出す。
	// ここでのX/Yは「入力の横/縦」であり、UEのワールド座標のX/Yではない。
	// 例：前=(0, 1)、右=(1, 0)、後ろ=(0, -1)になるようにMapping Context側で設定する。
	FVector2D MovementVector = Values.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// 操作上の向きを取得する。キャラクターの見た目やカメラの実際の回転と常に同じとは限らない。
		const FRotator Rotation = Controller->GetControlRotation();
		// 地面に沿って移動したいので、上下の傾きと横倒しを捨て、左右の向き（Yaw）だけを使う。
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		
		// UEのローカル軸はX=前、Y=右、Z=上。
		// FRotationMatrixはその軸をYaw方向へ回し、GetUnitAxisでワールド空間の長さ1の方向を取り出す。
		// EAxis::X/Yは回転させる前のローカル軸を選ぶ指定であり、MovementVector.X/Yとは別の意味。
		const FVector ForwardVector = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightVector = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// 入力の縦Yを「前方向」に、入力の横Xを「右方向」に掛ける。XとYを間違えて交換しているわけではない。
		// 考え方は ForwardVector * MovementVector.Y + RightVector * MovementVector.X。
		// 例：Yaw=0で前入力(0, 1)なら、UEの前方向であるワールド+Xへ移動入力を加える。
		// 第2引数は方向に掛ける入力の強さ。0.5なら半分の入力、-1なら逆方向の入力になる。
		// Transformを直接動かすのではなく入力を蓄積し、CharacterMovementが速度や衝突を扱って移動する。
		// この値は移動距離ではないので、ここでDeltaTimeを掛ける必要はない。
		AddMovementInput(ForwardVector, MovementVector.Y);
		AddMovementInput(RightVector, MovementVector.X);
		
	}
}

void ARsCharacter::Look(const FInputActionValue& Values)
{
	// 視点用Actionの2D値。Yの正負や感度はMapping ContextのModifierなどの設定にも左右される。
	FVector2D LookDirection = Values.Get<FVector2D>();
	
	if (Controller != nullptr)
	{
		// 横入力X → 左右回転Yaw、縦入力Y → 上下回転Pitch。位置ではなくControllerの操作方向を変える。
		// カメラがその方向を向くには、スプリングアーム等がControl Rotationを使う設定になっている必要がある。
		AddControllerYawInput(LookDirection.X);
		AddControllerPitchInput(LookDirection.Y);
	}
}

