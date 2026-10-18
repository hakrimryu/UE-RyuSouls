#include "Equipments/RsEquipment.h"

#include "GameFramework/Character.h"

ARsEquipment::ARsEquipment()
{
	PrimaryActorTick.bCanEverTick = true;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("EquipmentMesh"));
	SetRootComponent(Mesh);
	// 装備中のメッシュが所有者や周囲の移動を妨げないよう、衝突を無効にする。
	Mesh->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
}

void ARsEquipment::BeginPlay()
{
	Super::BeginPlay();

}

void ARsEquipment::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ARsEquipment::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	// BPで指定されたアセットを反映する。未指定の場合は現在の表示を変更しない。
	if (MeshAsset)
	{
		Mesh->SetStaticMesh(MeshAsset);
	}
}

void ARsEquipment::EquipItem()
{
}

void ARsEquipment::UnEquipItem()
{
}

void ARsEquipment::AttachToOwner(FName SocketName)
{
	// ActorのOwnerを取り付け先として使うため、キャラクターとそのMeshを順に確認する。
	if (ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner()))
	{
		if (USkeletalMeshComponent* CharacterMesh = OwnerCharacter->GetMesh())
		{
			// 装備の位置・回転・スケールを指定ソケットに合わせる。
			AttachToComponent(CharacterMesh, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), SocketName);
		}
	}
}
