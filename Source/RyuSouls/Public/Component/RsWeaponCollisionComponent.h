#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "RsWeaponCollisionComponent.generated.h"

// 新しく検出したActorの命中情報を武器側のダメージ処理へ渡す。
DECLARE_MULTICAST_DELEGATE_OneParam(FOnHitActor, const FHitResult&);

// 攻撃の有効区間中に武器の2ソケット間を球で検索し、同じActorへの重複通知を抑える。
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class RYUSOULS_API URsWeaponCollisionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// 1つの有効区間につき各Actorへ1回だけ通知する命中イベント。
	FOnHitActor OnHitActor;

protected:
	// トレース開始位置を取得する武器メッシュ上のソケット名。武器BPで指定する。
	UPROPERTY(EditAnywhere)
	FName TraceStartSocketName;

	// トレース終了位置を取得する武器メッシュ上のソケット名。
	UPROPERTY(EditAnywhere)
	FName TraceEndSocketName;

protected:
	// ソケット間を検索する球の半径（cm）。
	UPROPERTY(EditAnywhere)
	float TraceRadius = 20.0f;

	// 検索対象のObject Type。コンストラクタではPawnを登録する。
	UPROPERTY(EditAnywhere)
	TArray <TEnumAsByte<EObjectTypeQuery>> TraceObjectTypes;

	// 所有キャラクターなど、球トレースの候補から除外するActor。
	UPROPERTY(EditAnywhere)
	TArray<AActor*> IgnoredActors;

	// トレース範囲の確認用描画。武器BPで表示方法を切り替えられる。
	UPROPERTY(EditAnywhere)
	TEnumAsByte<EDrawDebugTrace::Type> DrawDebugType = EDrawDebugTrace::ForDuration;

protected:
	// ソケット位置の取得元。装備時に武器のMeshComponentを設定する。
	UPROPERTY()
	UPrimitiveComponent* WeaponMesh;

	// 今回の有効区間ですでに命中したActor。次の区間開始時に消去する。
	UPROPERTY()
	TArray<AActor*> AlreadyHitActors;

	// Notify区間内だけTickでトレースするための実行フラグ。
	bool bIsCollisionEnabled = false;

public:
	URsWeaponCollisionComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	// 命中履歴を消去し、新しい攻撃区間のトレースを開始する。
	void TurnOnCollision();

	// Notify区間の終了時にトレースを停止する。
	void TurnOffCollision();

	// トレース開始前に、ソケットを持つ武器メッシュを登録する。
	void SetWeaponMesh(UPrimitiveComponent* MeshComponent);

	// 所有キャラクターなどをトレース対象から除外する。
	void AddIgnoredActor(AActor* Actor);

	// 指定Actorを除外リストから取り除く。
	void RemoveIgnoredActor(AActor* Actor);

protected:
	// 同じ有効区間でまだ命中していないActorかを確認する。
	bool CanHitActor(AActor* Actor) const;

	// 現在の2ソケット位置を結ぶ球トレースで対象を列挙し、新規の命中だけを通知する。
	void CollisionTrace();
};
