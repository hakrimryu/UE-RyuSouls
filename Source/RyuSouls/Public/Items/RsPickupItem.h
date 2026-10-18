
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/RsInteract.h"
#include "RsPickupItem.generated.h"

class ARsEquipment;
// ワールド上に落ちている装備。対応する装備クラスのメッシュを表示し、操作時に所有者付きの装備Actorへ置き換える。
UCLASS()
class RYUSOULS_API ARsPickupItem : public AActor, public IRsInteract
{
	GENERATED_BODY()

public:
	ARsPickupItem();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	// 装備クラスの既定メッシュを表示し、落下用の物理シミュレーションを有効にする。
	virtual void OnConstruction(const FTransform& Transform) override;

	// InteractionActorをOwnerにした装備を生成し、生成できた場合に装備処理を呼んで自身を破棄する。
	virtual void Interact(AActor* InteractionActor) override;

public:
	// 武器交換時の遅延生成で、FinishSpawningより前に表示と再取得に使う装備クラスを設定する。
	FORCEINLINE void SetEquipmentClass(const TSubclassOf<ARsEquipment>& NewEquipmentClass) { EquipmentClass = NewEquipmentClass; }

protected:
	// 表示・検索・物理シミュレーションを兼ねるルートMesh。メッシュは装備クラス、衝突応答はコンストラクタで設定する。
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category= "Item")
	UStaticMeshComponent* Mesh;

	// 表示の参照元であり、取得時に生成する装備のBPクラス。レベル上の各Pickupで種類を選ぶ。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	TSubclassOf<ARsEquipment> EquipmentClass;

};
