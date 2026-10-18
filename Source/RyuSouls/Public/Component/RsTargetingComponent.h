#pragma once

#include "CoreMinimal.h"
#include "RsDefine.h"
#include "Components/ActorComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "RsTargetingComponent.generated.h"

class UCameraComponent;

// 専用チャンネルの候補検索・左右の切り替え・ロック中のカメラと旋回方式を管理する。
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class RYUSOULS_API URsTargetingComponent : public UActorComponent
{
	GENERATED_BODY()

protected:
	// 候補検索とロック継続に使う距離上限（cm）。既定値1000cmは10m。
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float TargetingRadius = 1000.0f;

	// RInterpToへ渡すカメラの追従補間速度。角速度そのものではない。
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float FaceLockOnRotationSpeed = 20.0f;

	// 候補検索の球と視線確認のラインに使うデバッグ描画方式。
	UPROPERTY(EditAnywhere)
	TEnumAsByte<EDrawDebugTrace::Type> DrawDebugType = EDrawDebugTrace::ForDuration;

protected:
	// 検索の中心と旋回設定の変更先。BeginPlayで所有Actorから取得する。
	UPROPERTY()
	ACharacter* Character;

	// 候補の正面度合いと左右の判定に使う、所有キャラクターのカメラ。
	UPROPERTY()
	UCameraComponent* Camera;

	// 現在のロック対象。対象側への表示通知とカメラの追従に使う。
	UPROPERTY()
	AActor* LockedTargetActor;

	// ロック中のカメラ更新と、キャラクター側の手動視点入力の抑止に使う。
	bool bIsLockOn = false;

public:
	URsTargetingComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	// 未ロックなら候補を検索して開始し、ロック中なら対象表示と旋回方式を元へ戻す。
	void ToggleLockOn();

	// 現在の対象を除いて左右の候補を探す。候補がなければロックを解除する。
	void SwitchingLockedOnActor(ESwitchingDirection InDirection);

	// キャラクターのLook処理が手動視点入力を受け付けるかを判断するための状態。
	FORCEINLINE bool IsLockOn() const { return bIsLockOn; }

protected:
	// 専用Object Typeの球検索から、インターフェースが対象可能と答えるActorを列挙する。
	void FindTargets(OUT TArray<AActor*>& OutTargetingActors) const;

	// 視線トレースのHitから、指定した左右条件を満たしカメラ正面との内積が最大のActorを選ぶ。
	AActor* FindClosestTarget(TArray<AActor*>& InTargets, ESwitchingDirection InDirection = ESwitchingDirection::None) const;

	// 移動方向への自動旋回を止め、Controllerが向く方向への旋回を有効にする。
	void OrientCamera() const;

	// Controller追従の旋回を止め、移動方向への自動旋回に戻す。
	void OrientMovement() const;

	// 対象の位置へControl RotationのPitch・Yawを補間し、Rollは維持する。
	void FaceLockOnActor() const;

	// 候補を選んでロックを開始し、対象側の表示とキャラクターの旋回方式を切り替える。
	void LockOnTarget();

	// ロック状態と参照を消去し、対象表示を消して移動方向への旋回に戻す。
	void StopLockOn();
};
