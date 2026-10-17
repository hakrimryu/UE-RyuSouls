#include "UI/RsPlayerHUDWidget.h"

#include "Component/RsAttributeComponent.h"

URsPlayerHUDWidget::URsPlayerHUDWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void URsPlayerHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// HUDを所有するプレイヤーのPawnから属性を取得する。Pawnや属性がなければ購読しない。
	if (APawn* OwningPawn = GetOwningPlayerPawn())
	{
		if (URsAttributeComponent* Attribute = OwningPawn->GetComponentByClass<URsAttributeComponent>())
		{
			Attribute->OnAttributeChanged.AddUObject(this, &ThisClass::OnAttributeChanged);
			// 購読後に現在値を通知し、最初の消費・回復を待たずにゲージを初期化する。
			Attribute->BroadcastAttributeChanged(ERsAttributeType::Stamina);
		}
	}
}

void URsPlayerHUDWidget::OnAttributeChanged(ERsAttributeType AttributeType, float InValue)
{
	// 通知には残量そのものではなく比率が入る。属性種別に対応する子ウィジェットへ渡す。
	switch (AttributeType)
	{
	case ERsAttributeType::Stamina:
		StaminaBarWidget->SetRatio(InValue);
		break;

	case ERsAttributeType::Health:
		// Health表示は未実装のため、この通知ではゲージを更新しない。
		break;
	}
}
