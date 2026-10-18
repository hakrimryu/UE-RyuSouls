#include "Component/RsWeaponCollisionComponent.h"

URsWeaponCollisionComponent::URsWeaponCollisionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	// 武器の表示メッシュではなく、Pawnのコリジョンを命中対象として検索する。
	TraceObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));
}

void URsWeaponCollisionComponent::BeginPlay()
{
	Super::BeginPlay();
}

void URsWeaponCollisionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bIsCollisionEnabled)
	{
		CollisionTrace();
	}
}

void URsWeaponCollisionComponent::TurnOnCollision()
{
	// 別のNotify区間では同じ相手にも再び命中できるよう、直前の履歴を破棄する。
	AlreadyHitActors.Empty();
	bIsCollisionEnabled = true;
}

void URsWeaponCollisionComponent::TurnOffCollision()
{
	bIsCollisionEnabled = false;
}

void URsWeaponCollisionComponent::SetWeaponMesh(UPrimitiveComponent* MeshComponent)
{
	WeaponMesh = MeshComponent;
}

void URsWeaponCollisionComponent::AddIgnoredActor(AActor* Actor)
{
	IgnoredActors.Add(Actor);
}

void URsWeaponCollisionComponent::RemoveIgnoredActor(AActor* Actor)
{
	IgnoredActors.Remove(Actor);
}

bool URsWeaponCollisionComponent::CanHitActor(AActor* Actor) const
{
	return AlreadyHitActors.Contains(Actor) == false;
}

void URsWeaponCollisionComponent::CollisionTrace()
{
	TArray<FHitResult> OutHits;

	// 装備時に登録されたメッシュから、現在フレームのワールド座標を取得する。
	const FVector Start = WeaponMesh->GetSocketLocation(TraceStartSocketName);
	const FVector End = WeaponMesh->GetSocketLocation(TraceEndSocketName);

	// 半径はcm単位。簡易コリジョンで複数件を取得し、末尾のtrueで所有する武器Actor自身を除外する。
	bool const bHit = UKismetSystemLibrary::SphereTraceMultiForObjects(
		GetOwner(),
		Start,
		End,
		TraceRadius,
		TraceObjectTypes,
		false,
		IgnoredActors,
		DrawDebugType,
		OutHits,
		true);

	if (bHit)
	{
		for (const FHitResult& Hit : OutHits)
		{
			AActor* HitActor = Hit.GetActor();

			if (HitActor == nullptr)
			{
				// 命中情報に有効なActorがなければ、履歴登録とダメージ通知を行わない。
				continue;
			}

			if (CanHitActor(HitActor))
			{
				// 次フレームや同じ検索内の別Hitで重複しないよう、通知前に履歴へ登録する。
				AlreadyHitActors.Add(HitActor);

				// 武器が購読している場合に命中情報を渡し、ダメージ量の計算と適用を委ねる。
				if (OnHitActor.IsBound())
				{
					OnHitActor.Broadcast(Hit);
				}
			}
		}
	}
}
