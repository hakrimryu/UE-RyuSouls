#pragma once

// DefaultEngine.iniのinteractionに対応するObject Channel。設定側のスロット番号と一致させる。
#define COLLISION_OBJECT_INTERACTION ECC_GameTraceChannel1

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
