#include "Characters/RsCharacter.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Camera/CameraComponent.h"
#include "Component/RsAttributeComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"

ARsCharacter::ARsCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	
	// 視点を動かしてもキャラクターの体が直接追従しないようにし、体の向きは移動側で制御する。
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
	
	// 移動入力による加速度の方向へ体を向ける。カメラが向いている方向とは独立して旋回する。
	GetCharacterMovement()->bOrientRotationToMovement = true;
	// 体の左右旋回を最大500度/秒にする。移動速度やマウス感度ではなく、体が向きを変える速さ。
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
	
	// 通常移動の上限は500cm/秒、歩行時の制動減速度は2000cm/秒²。上限速度はスプリント時に切り替える。
	GetCharacterMovement()->MaxWalkSpeed = 500.0f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.0f;
	
	// キャラクターのルート → スプリングアーム → カメラ、という親子関係を作る。
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	// 標準のUE単位では400cm。FRotatorの引数はPitch（上下）、Yaw（左右）、Roll（傾き）の順。
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->SetRelativeRotation(FRotator(-30.0f, 0.0f, 0.0f));
	// 視点の回転はスプリングアームがControllerのControl Rotationに追従して受け持つ。
	// Pitchを継承する設定では、上の-30度を加算するのではなく、Control RotationのPitchを使う。
	CameraBoom->bUsePawnControlRotation = true;
	
	FollowBoom = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowBoom"));
	FollowBoom->SetupAttachment(CameraBoom);
	// カメラ自身はControl Rotationを直接使わず、親のスプリングアームの回転に従う。
	FollowBoom->bUsePawnControlRotation = false;
	
	// スタミナの残量と回復タイマーをキャラクターごとのコンポーネントで管理する。
	AttributeComponent = CreateDefaultSubobject<URsAttributeComponent>(	TEXT("Attribute"));

}

void ARsCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

void ARsCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	// スタミナと速度上限の確認用表示。同じキーのメッセージを毎フレーム更新する。
	GEngine->AddOnScreenDebugMessage(0, 1.5f, FColor::Cyan, FString::Printf(TEXT("Stamina: %f"), AttributeComponent->GetBaseStamina()));
	GEngine->AddOnScreenDebugMessage(2, 1.5f, FColor::Cyan, FString::Printf(TEXT("MaxWalkSpeed: %f"), GetCharacterMovement()->MaxWalkSpeed));
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
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ThisClass::Look);
		
		// 発火中はスプリント条件を確認し、Completedで通常速度とスタミナ回復へ戻す。
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Triggered, this, &ThisClass::Sprinting);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &ThisClass::StopSprint);
	}

}

bool ARsCharacter::IsMoving() const
{
	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		// 水平速度が3cm/秒を超え、移動入力による加速度もある場合に限る。惰性移動だけではスプリントしない。
		return MovementComponent->Velocity.Size2D() > 3.0f && MovementComponent->GetCurrentAcceleration() != FVector::Zero();
	}

	return false;
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

void ARsCharacter::Sprinting()
{
	// 5.0は継続を許可する残量の閾値で、1回の消費量ではない。移動中のみ回復を止めて速度上限を上げる。
	if (AttributeComponent->CheckHasEnoughStamina(5.0f) && IsMoving())
	{
		AttributeComponent->ToggleStaminaRegeneration(false);
		GetCharacterMovement()->MaxWalkSpeed = SprintingSpeed;
		// 消費量はこの関数の呼び出し1回につき0.1。秒単位ではなく、入力Actionの発火頻度に依存する。
		AttributeComponent->DecreaseStamina(0.1f);
	}
	else
	{
		StopSprint();
	}
}

void ARsCharacter::StopSprint()
{
	// 入力終了または継続条件を満たさない場合の共通処理。回復タイマーが未開始なら2秒後に回復を始める。
	GetCharacterMovement()->MaxWalkSpeed = NormalSpeed;
	AttributeComponent->ToggleStaminaRegeneration(true, 2.0f);
}

