#pragma once

#include "CoreMinimal.h"
#include "Equipments/RsWeapon.h"
#include "RsFistWeapon.generated.h"

// キャラクター自身のMeshを左右の手のトレース元として使う素手用武器。
UCLASS()
class RYUSOULS_API ARsFistWeapon : public ARsWeapon
{
	GENERATED_BODY()
	
public:
	ARsFistWeapon();
	
public:
	// キャラクターのMeshを主・補助判定へ登録し、素手戦闘を常時有効にする。
	virtual void EquipItem() override;
	
};
