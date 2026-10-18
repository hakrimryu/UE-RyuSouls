#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RsEquipment.generated.h"

// 表示メッシュと所有キャラクターのソケットへの取り付けを共通化する装備の基底クラス。
UCLASS()
class RYUSOULS_API ARsEquipment : public AActor
{
	GENERATED_BODY()

public:
	// BPや配置インスタンスで指定する表示用アセット。OnConstructionでMeshへ反映する。
	UPROPERTY(EditAnywhere, Category = "Equipment | Mesh")
	UStaticMesh* MeshAsset;

	// 装備の表示と取り付けを受け持つルートコンポーネント。
	UPROPERTY(VisibleAnywhere, Category = "Equipment | Mesh")
	UStaticMeshComponent* Mesh;

public:
	ARsEquipment();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	// 配置・生成時やBPの設定変更時に、指定アセットを表示へ反映する。
	virtual void OnConstruction(const FTransform& Transform) override;

public:
	// 派生クラスが装備時の処理を定義する入口。基底クラスでは何もしない。
	virtual void EquipItem();

	// 派生クラスが装備解除時の処理を定義する入口。基底クラスでは何もしない。
	virtual void UnEquipItem();

	// Ownerがキャラクターの場合、そのSkeletalMeshの指定ソケットへ取り付ける。
	virtual void AttachToOwner(FName SocketName);

};
