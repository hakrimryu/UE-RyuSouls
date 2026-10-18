#pragma once

// DefaultEngine.iniのinteractionに対応するObject Channel。設定側のスロット番号と一致させる。
#define COLLISION_OBJECT_INTERACTION ECC_GameTraceChannel1
// DefaultEngine.iniのtargetingに対応するObject Channel。敵の球と候補検索で共用する。
#define COLLISION_OBJECT_TARGETING ECC_GameTraceChannel2

// 属性変更通知で、HUDのどのゲージを更新するかを識別する。
UENUM(BlueprintType)
enum class ERsAttributeType : uint8
{
	Stamina,
	Health, // 被ダメージ時の属性通知に使う。現在のHUDでは表示を更新しない。
};

// 被弾したActor自身を基準とする攻撃者の方向。敵の被弾Montageの選択に使う。
UENUM(BlueprintType)
enum class EHitDirection : uint8
{
	Front,
	Back,
	Left,
	Right,
};

// ロック対象を選ぶ方向。Noneは初回選択、Left・Rightは現在の対象を除く左右の切り替え。
UENUM(BlueprintType)
enum class ESwitchingDirection : uint8
{
	None,
	Left,
	Right,
};
