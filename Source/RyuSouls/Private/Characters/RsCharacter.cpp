#include "Characters/RsCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"

ARsCharacter::ARsCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->SetRelativeRotation(FRotator(-30.0f, 0.0f, 0.0f));
	
	FollowBoom = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowBoom"));
	FollowBoom->SetupAttachment(CameraBoom);

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

