#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_RsAttackFinished.generated.h"

// 攻撃Montageの終了位置から移動復帰とコンボのリセット待ちを要求する。
UCLASS(meta=(DisplayName = "Attack Finished"))
class RYUSOULS_API UAnimNotify_RsAttackFinished : public UAnimNotify
{
	GENERATED_BODY()

protected:
	// 攻撃終了後に次の入力を待つ秒数。Montage上のNotifyごとに設定する。
	UPROPERTY(EditAnywhere)
	float ComboResetDelay = 1.f;

public:
	UAnimNotify_RsAttackFinished(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	// 所有キャラクターへ、攻撃終了とコンボ継続の猶予時間を渡す。
	virtual void Notify(USkeletalMeshComponent* MeshComponent, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
