#include "RsGameplayTags.h"

namespace RsGameplayTags
{
	// C++の共通タグをCharacter.State配下の名前で登録し、状態の設定と比較に同じ識別子を使う。
	UE_DEFINE_GAMEPLAY_TAG(Character_State_Attacking, "Character.State.Attacking");
	UE_DEFINE_GAMEPLAY_TAG(Character_State_Rolling, "Character.State.Rolling");
}
