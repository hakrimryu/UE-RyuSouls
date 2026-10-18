#include "Characters/RsEnemy.h"

#include "RsDefine.h"
#include "RsGameplayTags.h"
#include "Component/RsAttributeComponent.h"
#include "Component/RsStateComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Engine/DamageEvents.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Sound/SoundCue.h"

ARsEnemy::ARsEnemy()
{
	PrimaryActorTick.bCanEverTick = true;

	// 候補の球検索用Object Typeを設定し、カメラからのVisibilityトレースだけをBlockする。
	TargetingSphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("TargetingSphere"));
	TargetingSphereComponent->SetupAttachment(GetRootComponent());
	TargetingSphereComponent->SetCollisionObjectType(COLLISION_OBJECT_TARGETING);
	TargetingSphereComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	TargetingSphereComponent->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	// ロック対象の表示を原点から50cm上に置く。描画サイズ30×30の画面空間Widgetを既定で隠す。
	LockOnWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("LockOnWidgetComponent"));
	LockOnWidgetComponent->SetupAttachment(GetRootComponent());
	LockOnWidgetComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 50.0f));
	LockOnWidgetComponent->SetDrawSize(FVector2D(30.0f, 30.0f));
	LockOnWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
	LockOnWidgetComponent->SetVisibility(false);

	// カメラのトレースを敵のカプセルとメッシュで遮らないようにする。
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);

	AttributeComponent = CreateDefaultSubobject<URsAttributeComponent>(TEXT("Attribute"));
	StateComponent = CreateDefaultSubobject<URsStateComponent>("State");

	// 体力コンポーネントの死亡通知から、この敵のRagdollへの切り替えを呼び出す。
	AttributeComponent->OnDeath.AddUObject(this, &ThisClass::OnDeath);
}

void ARsEnemy::BeginPlay()
{
	Super::BeginPlay();
}

void ARsEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

float ARsEnemy::TakeDamage(float Damage, const FDamageEvent& DamageEvent, AController* EventInstigator,
	AActor* DamageCauser)
{
	float  ActualDamage = Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);

	if (AttributeComponent)
	{
		// 体力を減らし、0に達した場合の死亡通知と状態更新はコンポーネントに委ねる。
		AttributeComponent->TakeDamageAmount(ActualDamage);
		GEngine->AddOnScreenDebugMessage(0, 1.5f, FColor::Cyan, FString::Printf(TEXT("Damaged : %f"), ActualDamage));
	}

	if (DamageEvent.IsOfType(FPointDamageEvent::ClassID))
	{
		// PointDamageの場合だけ、接触位置を含む専用イベントとして読み取る。
		const FPointDamageEvent* PointDamageEvent = static_cast<const FPointDamageEvent*>(&DamageEvent);

		// 武器側から渡された攻撃方向。
		FVector ShotDirection = PointDamageEvent->ShotDirection;
		// 衝突面の接触位置。命中音とパーティクルの生成位置に使う。
		FVector ImpactPoint = PointDamageEvent->HitInfo.ImpactPoint;
		// 衝突面の法線方向。
		FVector ImpactDirection = PointDamageEvent->HitInfo.ImpactNormal;
		// トレース形状が衝突した時点の位置。接触点のImpactPointとは区別する。
		FVector HitLocation = PointDamageEvent->HitInfo.Location;

		ImpactEffect(ImpactPoint);

		// ダメージを発生させたControllerのPawnを基準に、被弾方向を選ぶ。
		HitReaction(EventInstigator->GetPawn());
	}

	return ActualDamage;
}

void ARsEnemy::OnDeath()
{
	if (UCapsuleComponent* CapsuleComp = GetCapsuleComponent())
	{
		// 死亡後に移動用カプセルが周囲を遮らないよう、衝突を無効にする。
		CapsuleComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	// メッシュ側はRagdoll用の衝突を使い、カメラを遮らず物理シミュレーションへ移行する。
	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		MeshComp->SetCollisionProfileName("Ragdoll");
		MeshComp->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
		MeshComp->SetSimulatePhysics(true);
	}
}

void ARsEnemy::ImpactEffect(const FVector& Location)
{
	if (ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), ImpactSound, Location);
	}

	if (ImpactParticle)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticle, Location);
	}
}

void ARsEnemy::HitReaction(const AActor* Attacker)
{
	if (UAnimMontage* HitReactAnimMontage = GetHitReactAnimation(Attacker))
	{
		float DelaySeconds = PlayAnimMontage(HitReactAnimMontage);
	}
}

UAnimMontage* ARsEnemy::GetHitReactAnimation(const AActor* Attacker) const
{
	// 自身の位置から攻撃者へ向く回転を求める。
	const FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), Attacker->GetActorLocation());
	// 自身の回転からLookAt回転を引き、各軸の角度差を-180〜180度へ正規化する。
	const FRotator DeltaRotation = UKismetMathLibrary::NormalizedDeltaRotator(GetActorRotation(), LookAtRotation);
	// 水平方向だけを分類するためYawを使う。この引き算の順では正の角度が左、負が右になる。
	const float DeltaZ = DeltaRotation.Yaw;

	EHitDirection HitDirection = EHitDirection::Front;

	// 正面は±45度、左右は45〜135度、背面は±135度以遠。境界では先に一致した分岐を使う。
	if (UKismetMathLibrary::InRange_FloatFloat(DeltaZ, -45.f, 45.f))
	{
		HitDirection = EHitDirection::Front;
		UE_LOG(LogTemp, Log, TEXT("Front"));
	}
	else if (UKismetMathLibrary::InRange_FloatFloat(DeltaZ, 45.f, 135.f))
	{
		HitDirection = EHitDirection::Left;
		UE_LOG(LogTemp, Log, TEXT("Left"));
	}
	else if (UKismetMathLibrary::InRange_FloatFloat(DeltaZ, 135.f, 180.f)
		|| UKismetMathLibrary::InRange_FloatFloat(DeltaZ, -180.f, -135.f))
	{
		HitDirection = EHitDirection::Back;
		UE_LOG(LogTemp, Log, TEXT("Back"));
	}
	else if (UKismetMathLibrary::InRange_FloatFloat(DeltaZ, -135.f, -45.f))
	{
		HitDirection = EHitDirection::Right;
		UE_LOG(LogTemp, Log, TEXT("Right"));
	}

	UAnimMontage* SelectedMontage = nullptr;
	// 方向ごとにBPへ割り当てたMontageを返し、未設定なら呼び出し側は再生しない。
	switch (HitDirection)
	{
	case EHitDirection::Front:
		SelectedMontage = HitReactAnimFront;
		break;
	case EHitDirection::Back:
		SelectedMontage = HitReactAnimBack;
		break;
	case EHitDirection::Left:
		SelectedMontage = HitReactAnimLeft;
		break;
	case EHitDirection::Right:
		SelectedMontage = HitReactAnimRight;
		break;
	}

	return SelectedMontage;
}

void ARsEnemy::OnTargeted(bool bTargeted)
{
	if (LockOnWidgetComponent)
	{
		LockOnWidgetComponent->SetVisibility(bTargeted);
	}
}

bool ARsEnemy::CanBeTargeted()
{
	if (!StateComponent)
	{
		return false;
	}

	FGameplayTagContainer TagCheck;
	TagCheck.AddTag(RsGameplayTags::Character_State_Death);

	return StateComponent->IsCurrentStateEqualToAny(TagCheck) == false;
}
