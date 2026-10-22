#pragma once

#include "CoreMinimal.h"
#include "RsDefine.h"
#include "UObject/Interface.h"
#include "RsCombatInterface.generated.h"

UINTERFACE()
class URsCombatInterface : public UInterface
{
	GENERATED_BODY()
};

// AnimNotifyから装備中の武器判定を切り替えるための共通窓口。
class RYUSOULS_API IRsCombatInterface
{
	GENERATED_BODY()

public:
	// 指定された武器判定の攻撃区間を開始する。
	virtual void ActivateWeaponCollision(EWeaponCollisionType WeaponCollisionType) = 0;

	// 指定された武器判定の攻撃区間を終了する。
	virtual void DeactivateWeaponCollision(EWeaponCollisionType WeaponCollisionType) = 0;
};
