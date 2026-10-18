#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "RsMontageActionData.generated.h"

// 1つのアクションタグに対応するMontage群。同じタグ内でIndexにより選択できる。
USTRUCT(BlueprintType)
struct FRsMontageGroup
{
	GENERATED_BODY()

public:
	// 呼び出し側が指定するIndex順にMontageを格納する。
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<UAnimMontage*> Animations;

};

// 武器BPから参照し、GameplayTagとMontage群の対応をエディタで設定するDataAsset。
UCLASS()
class RYUSOULS_API URsMontageActionData : public UPrimaryDataAsset
{
	GENERATED_BODY()

protected:
	// DA_SwordAndShieldでは装備・解除タグにそれぞれ対応するMontageを登録する。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, DisplayName= "montage Groups")
	TMap<FGameplayTag, FRsMontageGroup> MontageGroupMap;

public:
	// タグとIndexに対応するMontageを返す。タグ未登録やIndexが配列上限以上の場合はnullptr。
	UAnimMontage* GetMontageForTag(const FGameplayTag& GroupTag, const int32 Index);

};
