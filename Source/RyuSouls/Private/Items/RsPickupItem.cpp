#include "Items/RsPickupItem.h"

ARsPickupItem::ARsPickupItem()
{
	PrimaryActorTick.bCanEverTick = true;

	// BPで指定したメッシュのコリジョンを、キャラクターの対象検索に使う。
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PickupItemMesh"));
	SetRootComponent(Mesh);
}

void ARsPickupItem::BeginPlay()
{
	Super::BeginPlay();

}

void ARsPickupItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ARsPickupItem::Interact(AActor* Interactor)
{
	// 毎フレーム更新されるステータス表示に上書きされないよう、独立したメッセージとして追加する。
	GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Cyan, TEXT("Interacted with pickup item!"));
}
