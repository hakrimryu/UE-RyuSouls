#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RsCombatComponent.generated.h"

class ARsWeapon;

// キャラクターの装備管理用コンポーネント。現在は主武器の参照を保持する。
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
	// 主武器の参照だけを更新する。生成やソケットへの取り付けは行わない。
	void SetWeapon(ARsWeapon* NewWeapon);
};
