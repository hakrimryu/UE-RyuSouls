#include "Data/RsMontageActionData.h"

UAnimMontage* URsMontageActionData::GetMontageForTag(const FGameplayTag& GroupTag, const int32 Index)
{
	// 未登録のタグでMapに要素を追加しないよう、登録済みのグループだけを参照する。
	if (MontageGroupMap.Contains(GroupTag))
	{
		const FRsMontageGroup& MontageGroup = MontageGroupMap[GroupTag];

		// Montageが登録され、指定Indexが配列上限未満の場合に候補を返す。
		if (MontageGroup.Animations.Num() > 0 && MontageGroup.Animations.Num() > Index)
		{
			return MontageGroup.Animations[Index];
		}
	}

	return nullptr;
}
