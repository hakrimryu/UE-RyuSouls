#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RsStatBarWidget.generated.h"

class UProgressBar;

// 残量比率と表示色を受け取り、UMGのProgressBarへ反映するゲージの基底クラス。
UCLASS()
class RYUSOULS_API URsStatBarWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	// WBP_StatBar内の同名のProgressBarとBindWidgetで接続する。
	UPROPERTY(meta = (BindWidget), BlueprintReadWrite)
	UProgressBar* StatBar;

	// ゲージの塗りつぶし色。BPや親HUDに配置したウィジェットで用途ごとに指定する。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StatBar")
	FLinearColor FillColorAndOpacity = FLinearColor::Red;

public:
	URsStatBarWidget(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

public:
	// デザイナー表示時と実行時の構築前に、指定した色をバーへ反映する。
	virtual void NativePreConstruct() override;

public:
	// 0.0から1.0の残量比率を受け取り、バーの充填率を更新する。
	void SetRatio(float Ratio) const;
};
