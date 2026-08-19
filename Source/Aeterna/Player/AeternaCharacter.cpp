// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/AeternaCharacter.h"

#include "Aeterna.h"
#include "Interaction/AeternaInteractableActor.h"
#include "Interaction/AeternaInteractableInterface.h"
#include "Player/Components/AeternaBatteryComponent.h"
#include "Player/Components/AeternaHeadBobComponent.h"
#include "Player/Components/AeternaInteractionComponent.h"
#include "Player/Components/AeternaInteractionPromptComponent.h"
#include "Player/Components/AeternaScanProgressComponent.h"

#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SpotLightComponent.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "InputActionValue.h"

AAeternaCharacter::AAeternaCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);
	
	// Create the first person mesh that will be viewed only by this character's owner
	FirstPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("First Person Mesh"));

	FirstPersonMesh->SetupAttachment(GetMesh());
	FirstPersonMesh->SetOnlyOwnerSee(true);
	FirstPersonMesh->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::FirstPerson;
	FirstPersonMesh->SetCollisionProfileName(FName("NoCollision"));

	// Create the Camera Component	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("First Person Camera"));
	FirstPersonCameraComponent->SetupAttachment(FirstPersonMesh, FName("head"));
	FirstPersonCameraComponent->SetRelativeLocationAndRotation(FVector(-2.8f, 5.89f, 0.0f), FRotator(0.0f, 90.0f, -90.0f));
	FirstPersonCameraComponent->bUsePawnControlRotation = true;
	FirstPersonCameraComponent->bEnableFirstPersonFieldOfView = true;
	FirstPersonCameraComponent->bEnableFirstPersonScale = true;
	FirstPersonCameraComponent->FirstPersonFieldOfView = 70.0f;
	FirstPersonCameraComponent->FirstPersonScale = 0.6f;

	HeadlampComponent = CreateDefaultSubobject<USpotLightComponent>(TEXT("Headlamp"));
	HeadlampComponent->SetupAttachment(FirstPersonCameraComponent);
	HeadlampComponent->SetRelativeLocation(FVector(18.0f, 0.0f, -4.0f));
	HeadlampComponent->SetRelativeRotation(FRotator::ZeroRotator);
	HeadlampComponent->SetVisibility(false);
	HeadlampComponent->SetUseTemperature(true);
	HeadlampComponent->SetInnerConeAngle(18.0f);
	HeadlampComponent->SetOuterConeAngle(36.0f);

	HeadBobComponent = CreateDefaultSubobject<UAeternaHeadBobComponent>(TEXT("HeadBobComponent"));
	BatteryComponent = CreateDefaultSubobject<UAeternaBatteryComponent>(TEXT("BatteryComponent"));
	InteractionComponent = CreateDefaultSubobject<UAeternaInteractionComponent>(TEXT("InteractionComponent"));
	InteractionPromptComponent = CreateDefaultSubobject<UAeternaInteractionPromptComponent>(TEXT("InteractionPromptComponent"));
	ScanProgressComponent = CreateDefaultSubobject<UAeternaScanProgressComponent>(TEXT("ScanProgressComponent"));

	// configure the character comps
	GetMesh()->SetOwnerNoSee(true);
	GetMesh()->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::WorldSpaceRepresentation;

	GetCapsuleComponent()->SetCapsuleSize(34.0f, 96.0f);

	// Configure character movement
	GetCharacterMovement()->MaxWalkSpeed = BasicWalkSpeed;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;
	GetCharacterMovement()->AirControl = 0.5f;
}

void AAeternaCharacter::BeginPlay()
{
	Super::BeginPlay();

	GetCharacterMovement()->MaxWalkSpeed = BasicWalkSpeed;
	if (HeadBobComponent)
	{
		HeadBobComponent->InitializePlayerComponent(this);
		HeadBobComponent->InitializeHeadBob(BasicSprintSpeed);
	}
	if (BatteryComponent)
	{
		BatteryComponent->InitializePlayerComponent(this);
		BatteryComponent->InitializeBattery(HeadlampComponent);
		BP_BatteryChanged(BatteryComponent->GetCurrentBattery(), BatteryComponent->GetMaxBattery(), BatteryComponent->GetBatteryNormalized());
	}
	if (InteractionComponent)
	{
		InteractionComponent->InitializePlayerComponent(this);
		InteractionComponent->InitializeInteraction(FirstPersonCameraComponent);
	}
	if (InteractionPromptComponent)
	{
		InteractionPromptComponent->InitializePlayerComponent(this);
	}
	if (ScanProgressComponent)
	{
		ScanProgressComponent->InitializePlayerComponent(this);
	}
}

void AAeternaCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UpdateHeadBob(DeltaSeconds);
	UpdateFocusedInteractable();

	if (BatteryComponent && BatteryComponent->TickBattery(DeltaSeconds, bHeadlampOn))
	{
		BP_BatteryChanged(BatteryComponent->GetCurrentBattery(), BatteryComponent->GetMaxBattery(), BatteryComponent->GetBatteryNormalized());

		if (bHeadlampOn && BatteryComponent->GetCurrentBattery() <= 0.0f)
		{
			bHeadlampOn = false;
			if (HeadlampComponent)
			{
				HeadlampComponent->SetVisibility(false);
			}

			UE_LOG(LogAeterna, Log, TEXT("Light down"));
			BP_HeadlampStateChanged(false);
		}
	}
}

void AAeternaCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{	
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AAeternaCharacter::DoJumpStart);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &AAeternaCharacter::DoJumpEnd);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AAeternaCharacter::MoveInput);

		// Sprinting
		if (BasicSprintAction)
		{
			EnhancedInputComponent->BindAction(BasicSprintAction, ETriggerEvent::Started, this, &AAeternaCharacter::StartBasicSprint);
			EnhancedInputComponent->BindAction(BasicSprintAction, ETriggerEvent::Completed, this, &AAeternaCharacter::EndBasicSprint);
		}

		// Notebook
		if (NotebookAction)
		{
			EnhancedInputComponent->BindAction(NotebookAction, ETriggerEvent::Started, this, &AAeternaCharacter::ToggleNotebook);
		}

		// Interaction
		if (InteractAction)
		{
			EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &AAeternaCharacter::TryInteract);
		}

		// Headlamp
		if (HeadlampAction)
		{
			EnhancedInputComponent->BindAction(HeadlampAction, ETriggerEvent::Started, this, &AAeternaCharacter::ToggleHeadlamp);
		}

		// Looking/Aiming
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AAeternaCharacter::LookInput);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &AAeternaCharacter::LookInput);
	}
	else
	{
		UE_LOG(LogAeterna, Error, TEXT("'%s' Failed to find an Enhanced Input Component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}


void AAeternaCharacter::MoveInput(const FInputActionValue& Value)
{
	// get the Vector2D move axis
	FVector2D MovementVector = Value.Get<FVector2D>();

	// pass the axis values to the move input
	DoMove(MovementVector.X, MovementVector.Y);

}

void AAeternaCharacter::LookInput(const FInputActionValue& Value)
{
	// get the Vector2D look axis
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	// pass the axis values to the aim input
	DoAim(LookAxisVector.X, LookAxisVector.Y);

}

void AAeternaCharacter::DoAim(float Yaw, float Pitch)
{
	if (GetController())
	{
		// pass the rotation inputs
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void AAeternaCharacter::DoMove(float Right, float Forward)
{
	if (GetController())
	{
		// pass the move inputs
		AddMovementInput(GetActorRightVector(), Right);
		AddMovementInput(GetActorForwardVector(), Forward);
	}
}

void AAeternaCharacter::DoJumpStart()
{
	// pass Jump to the character
	Jump();
}

void AAeternaCharacter::DoJumpEnd()
{
	// pass StopJumping to the character
	StopJumping();
}

void AAeternaCharacter::StartBasicSprint()
{
	bBasicSprinting = true;
	GetCharacterMovement()->MaxWalkSpeed = BasicSprintSpeed;
}

void AAeternaCharacter::EndBasicSprint()
{
	bBasicSprinting = false;
	GetCharacterMovement()->MaxWalkSpeed = BasicWalkSpeed;
}

bool AAeternaCharacter::IsSprinting() const
{
	return bBasicSprinting;
}

void AAeternaCharacter::ToggleNotebook()
{
	bNotebookOpen = !bNotebookOpen;
	const FString NotebookMessage = bNotebookOpen ? TEXT("Notebook open") : TEXT("Notebook close");

	UE_LOG(LogAeterna, Log, TEXT("%s"), *NotebookMessage);

	BP_NotebookStateChanged(bNotebookOpen);
}

void AAeternaCharacter::TryInteract()
{
	if (!InteractionComponent || !InteractionComponent->TryInteract(this))
	{
		UE_LOG(LogAeterna, Log, TEXT("No interaction target"));

		BP_InteractionFailed();
	}
}

void AAeternaCharacter::UpdateHeadBob(float DeltaSeconds)
{
	if (HeadBobComponent)
	{
		HeadBobComponent->UpdateHeadBob(DeltaSeconds, bBasicSprinting);
	}
}

void AAeternaCharacter::UpdateFocusedInteractable()
{
	if (InteractionComponent && InteractionComponent->UpdateFocusedInteractable(this))
	{
		AActor* NewFocusedActor = InteractionComponent->GetFocusedInteractableActor();
		const FAeternaInteractionInfo InteractionInfo = InteractionComponent->GetFocusedInteractionInfo();

		if (InteractionPromptComponent)
		{
			if (NewFocusedActor)
			{
				InteractionPromptComponent->ShowPrompt(NewFocusedActor, InteractionInfo);
			}
			else
			{
				InteractionPromptComponent->HidePrompt();
			}
		}

		BP_FocusedInteractableChanged(NewFocusedActor, InteractionInfo);
	}
}

void AAeternaCharacter::ToggleHeadlamp()
{
	if (!BatteryComponent)
	{
		return;
	}

	if (!bHeadlampOn && BatteryComponent->GetCurrentBattery() <= 0.0f)
	{
		UE_LOG(LogAeterna, Log, TEXT("Light unavailable: battery empty"));
		return;
	}

	bHeadlampOn = !bHeadlampOn;
	if (HeadlampComponent)
	{
		HeadlampComponent->SetVisibility(bHeadlampOn);
	}
	UpdateHeadlampBrightness();

	const FString HeadlampMessage = bHeadlampOn ? TEXT("Light on") : TEXT("Light down");

	UE_LOG(LogAeterna, Log, TEXT("%s"), *HeadlampMessage);

	BP_HeadlampStateChanged(bHeadlampOn);
}

void AAeternaCharacter::AddPlayerBattery(float Amount)
{
	if (!BatteryComponent || Amount <= 0.0f)
	{
		return;
	}

	BatteryComponent->AddBattery(Amount);
	BatteryComponent->UpdateBatteryDebugString(bHeadlampOn);
	BP_BatteryChanged(BatteryComponent->GetCurrentBattery(), BatteryComponent->GetMaxBattery(), BatteryComponent->GetBatteryNormalized());
}

bool AAeternaCharacter::RegisterScanPoint(FName ScanPointId)
{
	if (!ScanProgressComponent || !ScanProgressComponent->RegisterScanPoint(ScanPointId))
	{
		return false;
	}

	const int32 CurrentCount = ScanProgressComponent->GetCompletedScanCount();
	const int32 RequiredCount = ScanProgressComponent->GetRequiredScanCount();
	BP_ScanProgressChanged(CurrentCount, RequiredCount);

	UE_LOG(LogAeterna, Log, TEXT("Scan complete: %s (%d / %d)"), *ScanPointId.ToString(), CurrentCount, RequiredCount);

	if (RequiredCount > 0 && CurrentCount >= RequiredCount)
	{
		BP_AllRequiredScansCompleted();
	}

	return true;
}

bool AAeternaCharacter::HasScannedPoint(FName ScanPointId) const
{
	return ScanProgressComponent && ScanProgressComponent->HasScannedPoint(ScanPointId);
}

void AAeternaCharacter::UpdateHeadlampBrightness()
{
	if (BatteryComponent)
	{
		BatteryComponent->UpdateHeadlampBrightness();
	}
}

void AAeternaCharacter::UpdateBatteryDebugString()
{
	if (BatteryComponent)
	{
		BatteryComponent->UpdateBatteryDebugString(bHeadlampOn);
	}
}

bool AAeternaCharacter::IsNotebookOpen() const
{
	return bNotebookOpen;
}

bool AAeternaCharacter::IsHeadlampOn() const
{
	return bHeadlampOn;
}

float AAeternaCharacter::GetCurrentBattery() const
{
	return BatteryComponent ? BatteryComponent->GetCurrentBattery() : 0.0f;
}

float AAeternaCharacter::GetLastBatteryChargeAmount() const
{
	return BatteryComponent ? BatteryComponent->GetLastBatteryChargeAmount() : 0.0f;
}

FString AAeternaCharacter::GetBatteryDebugString() const
{
	return BatteryComponent ? BatteryComponent->GetBatteryDebugString() : FString();
}

float AAeternaCharacter::GetBatteryNormalized() const
{
	return BatteryComponent ? BatteryComponent->GetBatteryNormalized() : 0.0f;
}

int32 AAeternaCharacter::GetCompletedScanCount() const
{
	return ScanProgressComponent ? ScanProgressComponent->GetCompletedScanCount() : 0;
}

AActor* AAeternaCharacter::GetFocusedInteractableActor() const
{
	return InteractionComponent ? InteractionComponent->GetFocusedInteractableActor() : nullptr;
}

FAeternaInteractionInfo AAeternaCharacter::GetFocusedInteractionInfo() const
{
	return InteractionComponent ? InteractionComponent->GetFocusedInteractionInfo() : FAeternaInteractionInfo();
}
