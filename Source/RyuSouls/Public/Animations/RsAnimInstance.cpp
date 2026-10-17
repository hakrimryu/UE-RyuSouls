#include "Animations/RsAnimInstance.h"
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
}
