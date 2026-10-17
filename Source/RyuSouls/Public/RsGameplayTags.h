#pragma once

#include "NativeGameplayTags.h"

namespace RsGameplayTags
{
	// 攻撃中の行動状態を表す共通タグ。
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_State_Attacking);
	// ローリング中の行動状態。ARsCharacterが開始時に状態コンポーネントへ設定する。
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_State_Rolling);
}
