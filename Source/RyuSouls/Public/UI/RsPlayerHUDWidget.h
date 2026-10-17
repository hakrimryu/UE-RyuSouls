#pragma once

#include "CoreMinimal.h"
#include "RsDefine.h"
#include "RsStatBarWidget.h"
#include "Blueprint/UserWidget.h"
#include "RsPlayerHUDWidget.generated.h"

// 属性変更通知を受け取り、プレイヤーのステータスゲージへ反映するHUDの基底クラス。
UCLASS()
class RYUSOULS_API URsPlayerHUDWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	// WBP_PlayerHUD内の同名の子ウィジェットとBindWidgetで接続する、必須のスタミナゲージ。
	UPROPERTY(meta=(BindWidget), BlueprintReadWrite)
	URsStatBarWidget* StaminaBarWidget;

public:
	URsPlayerHUDWidget(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

public:
	// HUD構築時に操作中のPawnの属性変更を購読し、スタミナの初期表示を更新する。
	virtual void NativeConstruct() override;

protected:
	// 属性種別と残量比率を受け取り、対応するゲージへ渡す。
	void OnAttributeChanged(ERsAttributeType AttributeType, float InValue);

};
