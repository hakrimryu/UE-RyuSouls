#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "RsInteract.generated.h"

// C++で実装するインタラクションの共通契約。BPでは実装済みのC++クラスを継承して使用する。
UINTERFACE(MinimalAPI)
class URsInteract : public UInterface
{
	GENERATED_BODY()
};

class RYUSOULS_API IRsInteract
{
	GENERATED_BODY()

public:
	// 操作を受けた対象の処理。Interactorは操作を行ったActorで、現在はプレイヤーキャラクターを渡す。
	virtual void Interact(AActor* Interactor) = 0;
};
