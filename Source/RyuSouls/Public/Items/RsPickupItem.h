
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/RsInteract.h"
#include "RsPickupItem.generated.h"

// インタラクションの検出と呼び出しを確認するPickup。現在はデバッグ表示のみで、取得処理は行わない。
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

	// キャラクターの検索から呼ばれ、操作を受けたことを画面に表示する。
	virtual void Interact(AActor* Interactor) override;

protected:
	// 表示と検索用コリジョンを兼ねるルートMesh。BPでメッシュとinteractionのObject Typeを設定する。
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category= "Item")
	UStaticMeshComponent* Mesh;


};
