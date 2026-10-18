#include "Component/RsTargetingComponent.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Interface/RsTargeting.h"
#include "Kismet/KismetMathLibrary.h"

URsTargetingComponent::URsTargetingComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void URsTargetingComponent::BeginPlay()
{
	Super::BeginPlay();

	// 検索位置と移動設定の変更元を保持し、カメラの向きを使った候補選択に備える。
	Character = Cast<ACharacter>(GetOwner());
	if (Character)
	{
		Camera = Character->GetComponentByClass<UCameraComponent>();
	}
}

void URsTargetingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bIsLockOn == false)
	{
		return;
	}

	if (IsValid(Character) == false)
	{
		return;
	}

	if (IsValid(LockedTargetActor) == false)
	{
		return;
	}

	const float Distance = FVector::Distance(Character->GetActorLocation(), LockedTargetActor->GetActorLocation());

	if (IRsTargeting* Targeting = Cast<IRsTargeting>(LockedTargetActor))
	{
		// 死亡などで対象外になった場合や、距離上限（cm）を超えた場合は表示と旋回設定を戻す。
		if (Targeting->CanBeTargeted() == false || Distance > TargetingRadius)
		{
			StopLockOn();
		}
		else
		{
			FaceLockOnActor();
		}
	}
}

void URsTargetingComponent::ToggleLockOn()
{
	if (bIsLockOn)
	{
		StopLockOn();
	}
	else
	{
		LockOnTarget();
	}
}

void URsTargetingComponent::SwitchingLockedOnActor(ESwitchingDirection InDirection)
{
	if (::IsValid(LockedTargetActor))
	{
		if (IRsTargeting* Targeting = Cast<IRsTargeting>(LockedTargetActor))
		{
			// 切り替え前に、以前の対象のロック表示を消す。
			Targeting->OnTargeted(false);

			// 候補を再検索し、現在の対象を除いて指定方向の次の対象を選ぶ。
			TArray<AActor*> OutTargets;
			FindTargets(OutTargets);
			AActor* TargetActor = FindClosestTarget(OutTargets, InDirection);

			if (::IsValid(TargetActor))
			{
				if (IRsTargeting* NewTargeting = Cast<IRsTargeting>(TargetActor))
				{
					LockedTargetActor = TargetActor;
					NewTargeting->OnTargeted(true);
				}
			}
			else
			{
				StopLockOn();
			}
		}
	}
}

void URsTargetingComponent::FindTargets(TArray<AActor*>& OutTargetingActors) const
{
	TArray<FHitResult> OutHits;
	const FVector Start = Character->GetActorLocation();
	// 始点と終点を同じにして、移動経路ではなく現在位置を中心とする球内を検索する。
	const FVector End = Start;

	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	// DefaultEngine.iniのtargetingに対応するGameTraceChannel2だけを検索する。
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(COLLISION_OBJECT_TARGETING));

	TArray<AActor*> ActorsToIgnore;

	const bool bHit = UKismetSystemLibrary::SphereTraceMultiForObjects(
		GetOwner(),
		Start,
		End,
		TargetingRadius,
		ObjectTypes,
		false,
		ActorsToIgnore,
		DrawDebugType,
		OutHits,
		true);

	if (bHit)
	{
		for (const FHitResult& HitResult : OutHits)
		{
			AActor* HitActor = HitResult.GetActor();
			if (IRsTargeting* Targeting = Cast<IRsTargeting>(HitActor))
			{
				// 専用チャンネルに属していても、死亡中など対象側が拒否するActorは候補にしない。
				if (Targeting->CanBeTargeted())
				{
					OutTargetingActors.Add(HitActor);
				}
			}
		}
	}
}

AActor* URsTargetingComponent::FindClosestTarget(TArray<AActor*>& InTargets, ESwitchingDirection InDirection) const
{
	float TargetCompareValue = 0.f;
	AActor* ClosestTarget = nullptr;

	for (const AActor* TargetActor : InTargets)
	{
		if (InDirection != ESwitchingDirection::None && LockedTargetActor == TargetActor)
		{
			continue;
		}

		FHitResult OutHit;
		const FVector Start = Camera->GetComponentLocation();
		const FVector End = TargetActor->GetActorLocation();

		TArray<AActor*> ActorsToIgnore;

		// カメラから候補までVisibilityで検索する。対象の球もこのチャンネルをBlockする必要がある。
		const bool bHit = UKismetSystemLibrary::LineTraceSingle(
			GetOwner(),
			Start,
			End,
			UEngineTypes::ConvertToTraceType(ECC_Visibility),
			false,
			ActorsToIgnore,
			DrawDebugType,
			OutHit,
			true);

		if (bHit)
		{
			// カメラの右方向とHitの法線の内積で、切り替え先の左右を判定する。
			if (InDirection == ESwitchingDirection::Left)
			{
				// 法線は表面から外へ向くため、左候補には正の内積を要求する。
				if (FVector::DotProduct(Camera->GetRightVector(), OutHit.Normal) > 0.f == false)
				{
					continue;
				}
			}

			// 右候補には、カメラの右方向と法線の内積が負であることを要求する。
			if ( InDirection == ESwitchingDirection::Right)
			{
				if (FVector::DotProduct(Camera->GetRightVector(), OutHit.Normal) < 0.f == false)
				{
					continue;
				}
			}

			AActor* HitActor = OutHit.GetActor();

			// 距離ではなく、カメラ正面とキャラクターからHitActorへ向く単位ベクトルの内積を比較する。
			// 初期比較値が0なので前方半球だけが候補となり、正面に近いHitActorを優先する。
			const FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(Character->GetActorLocation(), HitActor->GetActorLocation());
			float CheckValue = FVector::DotProduct(Camera->GetForwardVector(), LookAtRotation.Vector());

			if (CheckValue > TargetCompareValue)
			{
				TargetCompareValue = CheckValue;
				ClosestTarget = HitActor;
			}
		}
	}

	return ClosestTarget;
}

void URsTargetingComponent::OrientCamera() const
{
	Character->GetCharacterMovement()->bOrientRotationToMovement = false;
	Character->GetCharacterMovement()->bUseControllerDesiredRotation = true;
}

void URsTargetingComponent::OrientMovement() const
{
	Character->GetCharacterMovement()->bOrientRotationToMovement = true;
	Character->GetCharacterMovement()->bUseControllerDesiredRotation = false;
}

void URsTargetingComponent::FaceLockOnActor() const
{
	const FRotator CurrentControlRotation = Character->GetControlRotation();

	// 対象の原点から150cm下を注視点とし、カメラのPitchも対象の高さに合わせる。
	const FVector TargetLocation = LockedTargetActor->GetActorLocation() - FVector(0.f, 0.f, 150.f);
	const FRotator TargetLookAtRotation = UKismetMathLibrary::FindLookAtRotation(Character->GetActorLocation(), TargetLocation);

	FRotator InterpRotation = FMath::RInterpTo(CurrentControlRotation, TargetLookAtRotation, GetWorld()->GetDeltaSeconds(), FaceLockOnRotationSpeed);

	// Rollは現在値のまま保ち、補間したPitchとYawだけをControllerへ反映する。
	Character->GetController()->SetControlRotation(FRotator(InterpRotation.Pitch, InterpRotation.Yaw, CurrentControlRotation.Roll));
}

void URsTargetingComponent::LockOnTarget()
{
	TArray<AActor*> OutTargets;
	FindTargets(OutTargets);
	AActor* TargetActor = FindClosestTarget(OutTargets);

	if (::IsValid(TargetActor))
	{
		LockedTargetActor = TargetActor;
		bIsLockOn = true;
		if (IRsTargeting* Targeting = Cast<IRsTargeting>(LockedTargetActor))
		{
			Targeting->OnTargeted(bIsLockOn);
		}

		OrientCamera();
	}
}

void URsTargetingComponent::StopLockOn()
{
	bIsLockOn = false;
	if (IRsTargeting* Targeting = Cast<IRsTargeting>(LockedTargetActor))
	{
		Targeting->OnTargeted(bIsLockOn);
	}
	LockedTargetActor = nullptr;
	OrientMovement();
}
