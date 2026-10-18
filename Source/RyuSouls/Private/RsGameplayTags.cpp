#include "RsGameplayTags.h"

namespace RsGameplayTags
{
	// C++の共通タグをCharacter.State配下の名前で登録し、状態の設定と比較に同じ識別子を使う。
	UE_DEFINE_GAMEPLAY_TAG(Character_State_Attacking, "Character.State.Attacking");
	UE_DEFINE_GAMEPLAY_TAG(Character_State_Rolling, "Character.State.Rolling");
	UE_DEFINE_GAMEPLAY_TAG(Character_State_GeneralAction, "Character.State.GeneralAction");

	// 行動状態とは別にアクションを識別し、DataAssetとAnimNotifyで同じ名前を使う。
	UE_DEFINE_GAMEPLAY_TAG(Character_Action_Equip, "Character.Action.Equip");
	UE_DEFINE_GAMEPLAY_TAG(Character_Action_Unequip, "Character.Action.Unequip");

}
