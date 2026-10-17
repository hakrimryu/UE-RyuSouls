#pragma once

// 属性変更通知で、HUDのどのゲージを更新するかを識別する。
UENUM(BlueprintType)
enum class ERsAttributeType : uint8
{
	Stamina,
	Health, // 識別子のみを用意しており、現在のHUDでは表示を更新しない。
};
