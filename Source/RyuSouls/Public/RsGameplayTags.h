#pragma once

#include "NativeGameplayTags.h"

namespace RsGameplayTags
{
	// 攻撃中の行動状態を表す共通タグ。
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_State_Attacking);
	// ローリング中の行動状態。ARsCharacterが開始時に状態コンポーネントへ設定する。
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_State_Rolling);
	// 抜刀・納刀の開始からResetState通知まで保持し、装備切り替えの重複を防ぐ。
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_State_GeneralAction);

	// DataAssetのMontage検索と、AnimNotifyの抜刀・納刀判定で共用するアクションタグ。
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_Action_Equip);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_Action_Unequip);
}
