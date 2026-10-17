
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "RsCharacter.generated.h"

UCLASS()
class RYUSOULS_API ARsCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ARsCharacter();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
