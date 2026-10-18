#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "RsTargeting.generated.h"

// ロック対象の表示切り替えと、現在対象にできるかの判定を共通化するインターフェース。
UINTERFACE(MinimalAPI)
class URsTargeting : public UInterface
{
	GENERATED_BODY()
};

class RYUSOULS_API IRsTargeting
{
	GENERATED_BODY()

public:
	// 対象に選ばれたときはtrue、解除・切り替え時はfalseでロック表示などを更新する。
	virtual void OnTargeted(bool bTargeted) = 0;

	// 死亡状態などを基に、候補検索とロック継続の対象にできるかを返す。
	virtual bool CanBeTargeted() = 0;
};
