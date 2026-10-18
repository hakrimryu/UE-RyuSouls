#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "AnimNotifyState_RsWeaponHit.generated.h"

// Montage上のWeapon Collision区間に合わせ、現在の主武器の命中判定を切り替える。
UCLASS(meta=(DisplayName="Weapon Collision"))
class RYUSOULS_API UAnimNotifyState_RsWeaponHit : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	UAnimNotifyState_RsWeaponHit(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	// 区間開始時に主武器の命中履歴を消去し、トレースを開始する。
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	// 区間終了時に主武器のトレースを停止する。
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
