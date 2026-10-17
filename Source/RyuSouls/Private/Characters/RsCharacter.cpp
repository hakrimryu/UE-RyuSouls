#include "Characters/RsCharacter.h"

ARsCharacter::ARsCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

}

void ARsCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

void ARsCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ARsCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

