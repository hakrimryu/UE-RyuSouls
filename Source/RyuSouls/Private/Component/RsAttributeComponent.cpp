#include "Component/RsAttributeComponent.h"

URsAttributeComponent::URsAttributeComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void URsAttributeComponent::BeginPlay()
{
	Super::BeginPlay();
}

void URsAttributeComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

bool URsAttributeComponent::CheckHasEnoughStamina(float StaminaCost) const
{
	return BaseStamina >= StaminaCost;
}

void URsAttributeComponent::DecreaseStamina(float StaminaCost)
{
	// 消費後の残量を0から最大値の範囲に収める。
	BaseStamina = FMath::Clamp(BaseStamina - StaminaCost, 0.0f, MaxStamina);

	// 更新後の残量を通知し、HUDのゲージを消費結果に合わせる。
	BroadcastAttributeChanged(ERsAttributeType::Stamina);
}

void URsAttributeComponent::ToggleStaminaRegeneration(bool bEnable, float StartDelay)
{
	if (bEnable)
	{
		// すでに開始待ち・回復中なら再登録しない。毎フレーム有効化されても回復開始が先延ばしにならない。
		if (GetWorld()->GetTimerManager().IsTimerActive(StaminaRegenTimerHandle) == false)
		{
			// StartDelay秒後に初回を実行し、以後は0.1秒ごとに回復する。回復量は1回あたりの値。
			GetWorld()->GetTimerManager().SetTimer(StaminaRegenTimerHandle, this, &ThisClass::RegenerateStaminaHandler, 0.1f, true, StartDelay);
		}
	}
	else
	{
		// スプリント中は、回復開始待ちも実行中の回復も取り消す。
		GetWorld()->GetTimerManager().ClearTimer(StaminaRegenTimerHandle);
	}
}

void URsAttributeComponent::BroadcastAttributeChanged(ERsAttributeType InAttributeType) const
{
	// HUDなどの購読先がある場合だけ、属性種別に対応する比率を計算して通知する。
	if (OnAttributeChanged.IsBound())
	{
		float Ratio = 0.0f;
		switch (InAttributeType)
		{
		case ERsAttributeType::Stamina:
			Ratio = GetStaminaRatio();
			break;

		case ERsAttributeType::Health:
			// Healthの比率計算は未実装のため、現在は初期値の0.0を通知する。
			break;
		}

		OnAttributeChanged.Broadcast(InAttributeType, Ratio);
	}
}

void URsAttributeComponent::RegenerateStaminaHandler()
{
	// 1回分の回復量を加算する。既定値0.2を0.1秒ごとに加えるため、回復中は秒あたり2.0になる。
	BaseStamina = FMath::Clamp(BaseStamina + StaminaRegenRate, 0.0f, MaxStamina);

	// 回復時も消費時と同じ通知を使い、HUD側で残量を定期取得する必要をなくす。
	BroadcastAttributeChanged(ERsAttributeType::Stamina);

	// 満タンになったらタイマーを止め、不要な回復処理を続けない。
	if (BaseStamina >= MaxStamina)
	{
		ToggleStaminaRegeneration(false);
	}
}
