#include "Characters/RsCharacter.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "RsDefine.h"
#include "RsGameplayTags.h"
#include "Blueprint/UserWidget.h"
#include "Camera/CameraComponent.h"
#include "Component/RsAttributeComponent.h"
#include "Component/RsStateComponent.h"
#include "Component/RsTargetingComponent.h"
#include "Equipments/RsWeapon.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Interface/RsInteract.h"
#include "Kismet/KismetSystemLibrary.h"
#include "UI/RsPlayerHUDWidget.h"

class IRsInteract;

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
	// 行動状態のタグと移動入力の許可をキャラクターごとに保持する。
	StateComponent = CreateDefaultSubobject<URsStateComponent>(TEXT("State"));

	// 装備側が主武器を登録できるよう、キャラクターごとに保持先を生成する。
	CombatComponent = CreateDefaultSubobject<URsCombatComponent>(TEXT("Combat"));

	// 対象選択とロック中のカメラ更新を、このキャラクターのコンポーネントに委ねる。
	TargetingComponent = CreateDefaultSubobject<URsTargetingComponent>(TEXT("Targeting"));
}

void ARsCharacter::BeginPlay()
{
	Super::BeginPlay();

	// BPで指定したHUDを開始時に生成する。クラス未指定時や生成失敗時は画面へ追加しない。
	if (PlayerHUDWidgetClass)
	{
		PlayerHUDWidget = CreateWidget<URsPlayerHUDWidget>(GetWorld(), PlayerHUDWidgetClass);
		if (PlayerHUDWidget)
		{
			PlayerHUDWidget->AddToViewport();
		}
	}

	// BPで指定した素手武器を生成し、初期状態から攻撃できる主武器として装備する。
	if (FistWeaponClass)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		ARsFistWeapon* FistWeapon = GetWorld()->SpawnActor<ARsFistWeapon>(FistWeaponClass, GetActorTransform(), SpawnParams);
		FistWeapon->EquipItem();
	}
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
		EnhancedInputComponent->BindAction(SprintRollingAction, ETriggerEvent::Triggered, this, &ThisClass::Sprinting);
		EnhancedInputComponent->BindAction(SprintRollingAction, ETriggerEvent::Completed, this, &ThisClass::StopSprint);
		// 同じActionのCanceledでローリングを開始する。通知の発生条件は入力アセットのTrigger設定に従う。
		EnhancedInputComponent->BindAction(SprintRollingAction, ETriggerEvent::Canceled, this, &ThisClass::Rolling);

		// Startedを使い、キーを押し続けても操作を毎フレーム繰り返さない。
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &ThisClass::Interact);

		// Rキーの押下開始ごとに抜刀・納刀を要求し、長押しによる連続切り替えを防ぐ。
		EnhancedInputComponent->BindAction(ToggleCombatAction, ETriggerEvent::Started, this, &ThisClass::ToggleCombat);

		// 押下開始時に抜刀を要求する。抜刀Montage中は攻撃の実行条件で入力を抑止する。
		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Started, this, &ThisClass::AutoToggleCombat);
		// IA_AttackのHold成立前に離した入力を、通常攻撃または走行攻撃として扱う。
		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Canceled, this, &ThisClass::Attack);
		// 0.7秒のHoldが成立すると特殊攻撃を1回だけ要求する。
		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Triggered, this, &ThisClass::SpecialAttack);
		// IA_ShiftとのChord条件を満たした入力開始時に強攻撃を要求する。
		EnhancedInputComponent->BindAction(HeavyAttackAction, ETriggerEvent::Started, this, &ThisClass::HeavyAttack);

		// 押下開始ごとにロックを切り替え、左右の入力で対象の再選択を要求する。
		EnhancedInputComponent->BindAction(LockOnTargetAction, ETriggerEvent::Started, this, &ThisClass::LockOnTarget);
		EnhancedInputComponent->BindAction(LeftTargetAction, ETriggerEvent::Started, this, &ThisClass::LeftTarget);
		EnhancedInputComponent->BindAction(RightTargetAction, ETriggerEvent::Started, this, &ThisClass::RightTarget);
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

bool ARsCharacter::CanToggleCombat() const
{
	check(StateComponent);

	// 主武器がない状態では、切り替え用Montageとソケットを参照できない。
	if (IsValid(CombatComponent->GetMainWeapon()) == false)
	{
		return false;
	}

	// 素手は生成時から戦闘状態を維持し、抜刀・納刀を行わない。
	if (CombatComponent->GetMainWeapon()->GetCombatType() == ECombatType::MeleeFists)
	{
		return false;
	}

	// 攻撃・ローリング・装備切り替えの途中では、新しい抜刀・納刀を開始しない。
	FGameplayTagContainer CheckTags;
	CheckTags.AddTag(RsGameplayTags::Character_State_Attacking);
	CheckTags.AddTag(RsGameplayTags::Character_State_Rolling);
	CheckTags.AddTag(RsGameplayTags::Character_State_GeneralAction);

	return StateComponent->IsCurrentStateEqualToAny(CheckTags) == false;
}

bool ARsCharacter::CanPerformAttack(const FGameplayTag& AttackTypeTag) const
{
	// 攻撃条件と消費量を参照するため、キャラクターに必須のコンポーネントを確認する。
	check(StateComponent)
	check(CombatComponent)
	check(AttributeComponent)

	// 武器未装備時はDataAssetとスタミナ消費量を参照できないため攻撃しない。
	if (IsValid(CombatComponent->GetMainWeapon()) == false)
	{
		return false;
	}

	FGameplayTagContainer CheckTags;
	CheckTags.AddTag(RsGameplayTags::Character_State_Rolling);
	CheckTags.AddTag(RsGameplayTags::Character_State_GeneralAction);

	const float StaminaCost = CombatComponent->GetMainWeapon()->GetStaminaCost(AttackTypeTag);

	// 攻撃中は除外せず、入力の予約可否はコンボ処理側で判定する。
	return StateComponent->IsCurrentStateEqualToAny(CheckTags) == false
		&& CombatComponent->IsCombatEnabled()
		&& AttributeComponent->CheckHasEnoughStamina(StaminaCost);
}

void ARsCharacter::Move(const FInputActionValue& Values)
{
	// 入力の許可判定に必須の、コンストラクタで生成したコンポーネントを確認する。
	check(StateComponent);

	// ローリングなどで入力を禁止している間は、新しい移動入力をCharacterMovementへ渡さない。
	if (StateComponent->MovementInputEnabled() == false)
	{
		return;
	}

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
	// ロック中の視点は対象への自動追従に任せ、手動の視点入力は受け付けない。
	if (TargetingComponent && TargetingComponent->IsLockOn())
	{
		return;
	}

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

		bSprinting = true;
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
	bSprinting = false;
}

void ARsCharacter::Rolling()
{
	// スタミナ消費と移動制限に必要な、生成済みコンポーネントを確認する。
	check(AttributeComponent);
	check(StateComponent);

	// 15.0はローリング1回分の消費量。残量が足りない場合は開始しない。
	if (AttributeComponent->CheckHasEnoughStamina(15.0f))
	{
		// 既存の回復タイマーを解除し、今回のローリングを基準に回復開始を設定し直す。
		AttributeComponent->ToggleStaminaRegeneration(false);

		// 移動入力の復帰はアニメーション側のResetMovementInput通知に委ねる。
		StateComponent->ToggleMovementInput(false);

		// 1回分をまとめて消費し、属性変更通知を通してHUDの残量にも反映する。
		AttributeComponent->DecreaseStamina(15.0f);

		// BPで割り当てたローリング用Montageを再生する。
		PlayAnimMontage(RollMontage);

		// 移動入力の復帰処理で解除するまで、現在の行動状態をローリングとして保持する。
		StateComponent->SetState(RsGameplayTags::Character_State_Rolling);

		// Montageの終了時刻とは独立して、1.5秒後からスタミナ回復を開始する。
		AttributeComponent->ToggleStaminaRegeneration(true, 1.5f);
	}
}

void ARsCharacter::Interact()
{
	FHitResult OutHit;
	// 始点と終点を同じにし、前方ではなくキャラクター位置を中心とした半径100cmの範囲を調べる。
	const FVector Start = GetActorLocation();
	const FVector End = Start;
	constexpr float Radius = 100.0f;

	// BPのMeshに設定したinteractionだけを対象にし、床などのWorldStaticは検索に含めない。
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(COLLISION_OBJECT_INTERACTION));

	TArray<AActor*> ActorsToIgnore;

	// 簡易コリジョンで1件を検出する。末尾のtrueは自身を除外し、デバッグ描画で検索範囲を確認する。
	bool bHit = UKismetSystemLibrary::SphereTraceSingleForObjects(
		this,
		Start,
		End,
		Radius,
		ObjectTypes,
		false,
		ActorsToIgnore,
		EDrawDebugTrace::ForDuration,
		OutHit,
		true);

	if (bHit)
	{
		if (AActor* HitActor = OutHit.GetActor())
		{
			// 専用Channelに属していても、C++のインターフェース未実装なら操作を呼び出さない。
			if (IRsInteract* Interaction = Cast<IRsInteract>(HitActor))
			{
				Interaction->Interact(this);
			}
		}
	}
}

void ARsCharacter::ToggleCombat()
{
	check(CombatComponent);
	check(StateComponent);

	if (CombatComponent)
	{
		if (const ARsWeapon* Weapon = CombatComponent->GetMainWeapon())
		{
			if (CanToggleCombat())
			{
				// 終端のResetState通知まで再入力を止める。武器の取り付け先は途中のEquip Weapon通知で変更する。
				StateComponent->SetState(RsGameplayTags::Character_State_GeneralAction);

				// 主武器のDataAssetから現在の抜刀状態に対応する逆側のMontageを取得する。
				if (CombatComponent->IsCombatEnabled())
				{
					PlayAnimMontage(Weapon->GetMontageForTag(RsGameplayTags::Character_Action_Unequip));
				}
				else
				{
					PlayAnimMontage(Weapon->GetMontageForTag(RsGameplayTags::Character_Action_Equip));
				}
			}
		}
	}
}

void ARsCharacter::AutoToggleCombat()
{
	// 抜刀の要求だけを行い、完了後に攻撃を自動再開する入力は保持しない。
	if (CombatComponent)
	{
		if (!CombatComponent->IsCombatEnabled())
		{
			ToggleCombat();
		}
	}
}

void ARsCharacter::Attack()
{
	const FGameplayTag AttackTypeTag = GetAttackPerform();

	if (CanPerformAttack(AttackTypeTag))
	{
		ExecuteComboAttack(AttackTypeTag);
	}
}

void ARsCharacter::SpecialAttack()
{
	const FGameplayTag AttackTypeTag = RsGameplayTags::Character_Attack_Special;

	if (CanPerformAttack(AttackTypeTag))
	{
		ExecuteComboAttack(AttackTypeTag);
	}
}

void ARsCharacter::HeavyAttack()
{
	AutoToggleCombat();

	const FGameplayTag AttackTypeTag = RsGameplayTags::Character_Attack_Heavy;

	if (CanPerformAttack(AttackTypeTag))
	{
		ExecuteComboAttack(AttackTypeTag);
	}
}

void ARsCharacter::LockOnTarget()
{
	TargetingComponent->ToggleLockOn();
}

void ARsCharacter::LeftTarget()
{
	TargetingComponent->SwitchingLockedOnActor(ESwitchingDirection::Left);
}

void ARsCharacter::RightTarget()
{
	TargetingComponent->SwitchingLockedOnActor(ESwitchingDirection::Right);
}

FGameplayTag ARsCharacter::GetAttackPerform() const
{
	if (IsSprinting())
	{
		return RsGameplayTags::Character_Attack_Running;
	}

	return RsGameplayTags::Character_Attack_Light;
}

void ARsCharacter::DoAttack(const FGameplayTag& AttackTypeTag)
{
	check(StateComponent)
	check(AttributeComponent)
	check(CombatComponent)

	if (const ARsWeapon* Weapon = CombatComponent->GetMainWeapon())
	{
		// 攻撃中は新しい移動入力を止め、Notifyで次段へ進む際の攻撃種別を保持する。
		StateComponent->SetState(RsGameplayTags::Character_State_Attacking);
		StateComponent->ToggleMovementInput(false);
		CombatComponent->SetLastAttackType(AttackTypeTag);

		AttributeComponent->ToggleStaminaRegeneration(false);

		UAnimMontage* Montage = Weapon->GetMontageForTag(AttackTypeTag, ComboCounter);
		if (!Montage)
		{
			// 段数に対応するMontageがなければ先頭へ戻す。タグ自体が未登録なら再取得もnullptrになる。
			ComboCounter = 0;
			Montage = Weapon->GetMontageForTag(AttackTypeTag, ComboCounter);
		}

		PlayAnimMontage(Montage);

		const float StaminaCost = Weapon->GetStaminaCost(AttackTypeTag);
		AttributeComponent->DecreaseStamina(StaminaCost);
		// 攻撃ごとに回復待ちを作り直し、1.5秒後からスタミナを回復する。
		AttributeComponent->ToggleStaminaRegeneration(true, 1.5f);
	}
}

void ARsCharacter::ExecuteComboAttack(const FGameplayTag& AttackTypeTag)
{
	if (StateComponent->GetCurrentState() != RsGameplayTags::Character_State_Attacking)
	{
		if (bComboSequenceRunning && bCanComboInput == false)
		{
			// 攻撃状態の解除後もリセット期限内なら、追加入力で次の段数へ進む。
			ComboCounter++;
			UE_LOG(LogTemp, Warning, TEXT("Additional input : Combo Counter = %d"), ComboCounter);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT(">>> ComboSequence Started <<<"));
			ResetCombo();
			bComboSequenceRunning = true;
		}

		DoAttack(AttackTypeTag);
		// 前の攻撃終了時に予約したリセットで、新しいコンボが途中終了するのを防ぐ。
		GetWorld()->GetTimerManager().ClearTimer(ComboResetTimerHandle);
	}
	else if (bCanComboInput)
	{
		// Notify区間内の入力は1回分だけ保持し、区間終了時に消費する。
		bSavedComboInput = true;
	}
}

void ARsCharacter::ResetCombo()
{
	UE_LOG(LogTemp, Warning, TEXT("Combo Reset"));

	bComboSequenceRunning = false;
	bCanComboInput = false;
	bSavedComboInput = false;
	ComboCounter = 0;
}

void ARsCharacter::EnableComboWindow()
{
	bCanComboInput = true;
	UE_LOG(LogTemp, Warning, TEXT("Combo Window Opened: Combo Counter = %d"), ComboCounter);
}

void ARsCharacter::DisableComboWindow()
{
	check(CombatComponent)

	bCanComboInput = false;

	if (bSavedComboInput)
	{
		// 予約時の入力種別には切り替えず、直前に実行した攻撃種別の次段を再生する。
		bSavedComboInput = false;
		ComboCounter++;
		UE_LOG(LogTemp, Warning, TEXT("Combo Window Closed: Advancing to next combo = %d"), ComboCounter);
		DoAttack(CombatComponent->GetLastAttackType());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Combo Window Closed: No input received"));
	}
}

void ARsCharacter::AttackFinished(const float ComboResetDelay)
{
	UE_LOG(LogTemp, Warning, TEXT("AttackFinished"));
	if (StateComponent)
	{
		// 遅延処理の完了時に移動入力と行動状態をまとめて復帰する。
		StateComponent->ToggleMovementInput(true);
	}
	// Notifyで指定した秒数だけ継続入力を待ち、期限が来たら段数と予約入力を消去する。
	GetWorld()->GetTimerManager().SetTimer(ComboResetTimerHandle, this, &ThisClass::ResetCombo, ComboResetDelay, false);
}

void ARsCharacter::ActivateWeaponCollision(EWeaponCollisionType WeaponCollisionType)
{
	if (CombatComponent)
	{
		CombatComponent->GetMainWeapon()->ActivateCollision(WeaponCollisionType);
	}
}

void ARsCharacter::DeactivateWeaponCollision(EWeaponCollisionType WeaponCollisionType)
{
	if (CombatComponent)
	{
		CombatComponent->GetMainWeapon()->DeactivateCollision(WeaponCollisionType);
	}
}
