#include "Animations/RsAnimInstance.h"

#include "KismetAnimationLibrary.h"
#include "Characters/RsCharacter.h"
#include "Component/RsStateComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

URsAnimInstance::URsAnimInstance()
{
}

void URsAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	// このアニメーションの所有Actorを取得する。ACharacterでなければCastの結果はnullptrになる。
	Character = Cast<ACharacter>(GetOwningActor());

	if (Character)
	{
		// 以降の更新で速度や落下状態を取得できるよう、移動コンポーネントを保持する。
		MovementComponent = Character->GetCharacterMovement();
	}
}

void URsAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	// 所有キャラクターを取得できていない間は、移動データを読み取らず更新を中断する。
	if (Character == nullptr)
	{
		return;
	}

	// 移動コンポーネントがなければ速度や移動状態を参照できないため、更新を中断する。
	if (MovementComponent == nullptr)
	{
		return;
	}

	// 上下方向も含めた、現在の実際の移動速度を取得する。
	Velocity = MovementComponent->Velocity;
	// Size2DはXとYだけで長さを計算する。落下速度で歩行・走行のブレンドが変わらないようにする。
	GroundSpeed = MovementComponent->Velocity.Size2D();

	// 水平速度が3cm/秒を超え、かつ移動加速度がゼロでないときだけ移動中と判定する。
	// 速度が残っていても加速度がゼロになればfalseになるため、減速完了を待たずに待機へ遷移できる。
	bShouldMove = GroundSpeed > 3.0f && MovementComponent->GetCurrentAcceleration() != FVector::ZeroVector;

	// Z方向の速度の正負ではなく、CharacterMovementの落下モードで判定する。ジャンプ上昇中もtrueになる。
	bIsFalling = MovementComponent->IsFalling();

	// ワールド速度をActorの向きから見た角度へ変換し、正面を保った前後左右移動のブレンドに使う。
    Direction = UKismetAnimationLibrary::CalculateDirection(Velocity, Character->GetActorRotation());
}

void URsAnimInstance::AnimNotify_ResetMovementInput()
{
	// 状態コンポーネントを持つARsCharacterが所有者の場合だけ復帰を要求する。
	if (ARsCharacter* LocalCharacter = Cast<ARsCharacter>(GetOwningActor()))
	{
		// Durationの既定値0秒で遅延処理を予約し、その完了時に移動許可と状態解除をまとめて行う。
		LocalCharacter->GetStateComponent()->ToggleMovementInput(true);
	}
}

void URsAnimInstance::AnimNotify_ResetState()
{
	// 抜刀・納刀の完了後にGeneralActionを解除し、次のRキー入力を受け付ける。
	if (ARsCharacter* LocalCharacter = Cast<ARsCharacter>(GetOwningActor()))
	{
		LocalCharacter->GetStateComponent()->ClearState();
	}
}
