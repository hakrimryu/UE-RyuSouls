#include "Component/RsCombatComponent.h"

URsCombatComponent::URsCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void URsCombatComponent::BeginPlay()
{
	Super::BeginPlay();
}

void URsCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void URsCombatComponent::SetWeapon(ARsWeapon* NewWeapon)
{
	// 武器から渡された参照を保持し、取り付け処理は装備側に任せる。
	MainWeapon = NewWeapon;
}
