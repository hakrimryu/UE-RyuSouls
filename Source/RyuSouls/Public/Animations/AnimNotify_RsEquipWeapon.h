#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_RsEquipWeapon.generated.h"

// Montage内の指定時刻に抜刀状態と武器ソケットを切り替える共通通知。
UCLASS(meta=(DisplayName="Equip Weapon"))
class RYUSOULS_API UAnimNotify_RsEquipWeapon : public UAnimNotify
{
	GENERATED_BODY()

protected:
	// 各通知でCharacter.Action.EquipまたはCharacter.Action.Unequipを指定する。
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag MontageActionTag;

public:
	UAnimNotify_RsEquipWeapon(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	// Meshの所有者から主武器を取得し、指定タグに対応する状態と取り付け先を反映する。
	virtual void Notify(USkeletalMeshComponent* MeshComponent, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
