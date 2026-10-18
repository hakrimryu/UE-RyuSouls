#include "Items/RsPickupItem.h"

#include "RsDefine.h"
#include "Equipments/RsEquipment.h"

ARsPickupItem::ARsPickupItem()
{
	PrimaryActorTick.bCanEverTick = true;

	// 装備クラスから反映するメッシュのコリジョンを、キャラクターの対象検索と落下に使う。
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PickupItemMesh"));
	SetRootComponent(Mesh);

	// C++から落とすPickupにも検索用Channelを設定し、カメラを遮らずPawnとは重なれるようにする。
	Mesh->SetCollisionObjectType(COLLISION_OBJECT_INTERACTION);
	Mesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	Mesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}

void ARsPickupItem::BeginPlay()
{
	Super::BeginPlay();

}

void ARsPickupItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ARsPickupItem::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (EquipmentClass)
	{
		// CDOは装備クラスの既定値を持つオブジェクト。表示だけのために装備Actorを生成しない。
		if (ARsEquipment* CDO = EquipmentClass->GetDefaultObject<ARsEquipment>())
		{
			Mesh->SetStaticMesh(CDO->MeshAsset);
			// メッシュの簡易コリジョンを使って重力で落下させる。装備中のActor側はNoCollisionのまま扱う。
			Mesh->SetSimulatePhysics(true);
		}
	}
}

void ARsPickupItem::Interact(AActor* InteractionActor)
{
	// 武器側がCombatComponentと取り付け先を取得できるよう、操作したActorを生成時からOwnerにする。
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = InteractionActor;

	ARsEquipment* SpawnItem = GetWorld()->SpawnActor<ARsEquipment>(EquipmentClass, GetActorTransform(), SpawnParams);

	// 生成できなかった場合はPickupを残す。成功時は生成した装備の処理を呼び、この物理Actorを取り除く。
	if (SpawnItem)
	{
		SpawnItem->EquipItem();
		Destroy();
	}

}
