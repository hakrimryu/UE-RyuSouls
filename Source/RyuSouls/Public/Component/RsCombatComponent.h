#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RsCombatComponent.generated.h"

class ARsWeapon;

// キャラクターの主武器を保持し、交換時には以前の武器をPickupとしてワールドへ戻す。
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class RYUSOULS_API URsCombatComponent : public UActorComponent
{
	GENERATED_BODY()

protected:
	// 装備側のSetWeapon呼び出しで登録される主武器。UPROPERTYで参照をGCに追跡させる。
	UPROPERTY()
	ARsWeapon* MainWeapon;

public:
	URsCombatComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	// 以前の主武器があれば同じ装備クラスのPickupへ置き換え、新しい参照を登録する。取り付けは武器側が行う。
	void SetWeapon(ARsWeapon* NewWeapon);
};
