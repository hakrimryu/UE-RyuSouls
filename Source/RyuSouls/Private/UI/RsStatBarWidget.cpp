#include "UI/RsStatBarWidget.h"

#include "Components/ProgressBar.h"

URsStatBarWidget::URsStatBarWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void URsStatBarWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	// デザイナー表示と実行時の両方で、BPや親HUDで指定した色を反映する。バー未取得時は更新しない。
	if (StatBar)
	{
		StatBar->SetFillColorAndOpacity(FillColorAndOpacity);
	}
}

void URsStatBarWidget::SetRatio(float Ratio) const
{
	// 0.0が空、1.0が満タン。属性側で求めた比率をProgressBarへ渡す。
	if (StatBar)
	{
		StatBar->SetPercent(Ratio);
	}
}
