// Copyright Epic Games, Inc. All Rights Reserved.

#include "AeternaCharacter.h"
#include "AeternaInteractableActor.h"
#include "AeternaInteractableInterface.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SpotLightComponent.h"
#include "Engine/Engine.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Aeterna.h"

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
	HeadlampComponent->SetIntensity(FullBatteryLightIntensity);
	HeadlampComponent->SetAttenuationRadius(FullBatteryAttenuationRadius);
	HeadlampComponent->SetUseTemperature(true);
	HeadlampComponent->SetTemperature(FullBatteryTemperature);
	HeadlampComponent->SetInnerConeAngle(18.0f);
	HeadlampComponent->SetOuterConeAngle(36.0f);

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
	CurrentBattery = FMath::Clamp(CurrentBattery, 0.0f, MaxBattery);
	UpdateHeadlampBrightness();
	UpdateBatteryDebugString();
	BP_BatteryChanged(CurrentBattery, MaxBattery, GetBatteryNormalized());
}

void AAeternaCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UpdateFocusedInteractable();

	if (bHeadlampOn && HeadlampBatteryDrainPerSecond > 0.0f)
	{
		CurrentBattery = FMath::Max(CurrentBattery - HeadlampBatteryDrainPerSecond * DeltaSeconds, 0.0f);
		UpdateHeadlampBrightness();
		UpdateBatteryDebugString();
		BP_BatteryChanged(CurrentBattery, MaxBattery, GetBatteryNormalized());

		BatteryDebugPrintTimer += DeltaSeconds;
		if (bShowBatteryDebugString && BatteryDebugPrintTimer >= BatteryDebugPrintInterval)
		{
			BatteryDebugPrintTimer = 0.0f;
			UE_LOG(LogAeterna, Log, TEXT("%s"), *BatteryDebugString);
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(1205, BatteryDebugPrintInterval + 0.1f, FColor::Orange, BatteryDebugString);
			}
		}

		if (CurrentBattery <= 0.0f)
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
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Cyan, NotebookMessage);
	}

	BP_NotebookStateChanged(bNotebookOpen);
}

void AAeternaCharacter::TryInteract()
{
	UpdateFocusedInteractable();

	AActor* HitActor = FocusedInteractableActor.Get();
	if (!HitActor)
	{
		UE_LOG(LogAeterna, Log, TEXT("No interaction target"));
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Silver, TEXT("No Interaction Target"));
		}

		BP_InteractionFailed();
		return;
	}

	if (AAeternaInteractableActor* NativeInteractable = Cast<AAeternaInteractableActor>(HitActor))
	{
		NativeInteractable->PerformInteraction(this);
		return;
	}

	IAeternaInteractableInterface::Execute_Interact(HitActor, this);
}

void AAeternaCharacter::UpdateFocusedInteractable()
{
	AActor* NewFocusedActor = nullptr;
	FAeternaInteractionInfo NewInteractionInfo;

	if (FirstPersonCameraComponent && GetWorld())
	{
		const FVector TraceStart = FirstPersonCameraComponent->GetComponentLocation();
		const FVector TraceEnd = TraceStart + FirstPersonCameraComponent->GetForwardVector() * InteractionTraceDistance;

		FHitResult HitResult;
		FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(AeternaInteractFocusTrace), false, this);

		if (GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, QueryParams))
		{
			AActor* HitActor = HitResult.GetActor();
			if (HitActor && HitActor->GetClass()->ImplementsInterface(UAeternaInteractableInterface::StaticClass()))
			{
				if (AAeternaInteractableActor* NativeInteractable = Cast<AAeternaInteractableActor>(HitActor))
				{
					NewInteractionInfo = NativeInteractable->GetInteractionInfo_Implementation(this);
				}
				else
				{
					NewInteractionInfo = IAeternaInteractableInterface::Execute_GetInteractionInfo(HitActor, this);
				}

				if (NewInteractionInfo.Type != EAeternaInteractionType::None || !NewInteractionInfo.PromptText.IsEmpty())
				{
					NewFocusedActor = HitActor;
				}
			}
		}
	}

	if (FocusedInteractableActor != NewFocusedActor)
	{
		FocusedInteractableActor = NewFocusedActor;
		FocusedInteractionInfo = NewInteractionInfo;
		BP_FocusedInteractableChanged(FocusedInteractableActor.Get(), FocusedInteractionInfo);

		if (GEngine)
		{
			if (FocusedInteractableActor)
			{
				const FText PromptText = FocusedInteractionInfo.PromptText.IsEmpty()
					? FText::FromString(TEXT("Interact"))
					: FocusedInteractionInfo.PromptText;
				const FString PromptMessage = FString::Printf(TEXT("[E] %s"), *PromptText.ToString());
				GEngine->AddOnScreenDebugMessage(-1, 0.5f, FColor::Green, PromptMessage);
			}
			else
			{
				GEngine->AddOnScreenDebugMessage(-1, 0.5f, FColor::Silver, TEXT("No Interaction"));
			}
		}
		return;
	}

	FocusedInteractionInfo = NewInteractionInfo;
}

void AAeternaCharacter::ToggleHeadlamp()
{
	if (!bHeadlampOn && CurrentBattery <= 0.0f)
	{
		UE_LOG(LogAeterna, Log, TEXT("Light unavailable: battery empty"));
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("Light unavailable: battery empty"));
		}
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
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, HeadlampMessage);
	}

	BP_HeadlampStateChanged(bHeadlampOn);
}

void AAeternaCharacter::AddPlayerBattery(float Amount)
{
	if (Amount <= 0.0f)
	{
		return;
	}

	const float PreviousBattery = CurrentBattery;
	CurrentBattery = FMath::Clamp(CurrentBattery + Amount, 0.0f, MaxBattery);
	LastBatteryChargeAmount = CurrentBattery - PreviousBattery;
	UpdateHeadlampBrightness();
	UpdateBatteryDebugString();
	BP_BatteryChanged(CurrentBattery, MaxBattery, GetBatteryNormalized());

	UE_LOG(LogAeterna, Log, TEXT("Battery charge complete: +%.1f -> %.1f / %.1f"), LastBatteryChargeAmount, CurrentBattery, MaxBattery);
	if (GEngine)
	{
		const FString BatteryMessage = FString::Printf(TEXT("Battery Charge Complete +%.0f | %.0f / %.0f"), LastBatteryChargeAmount, CurrentBattery, MaxBattery);
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, BatteryMessage);
		GEngine->AddOnScreenDebugMessage(1205, 2.0f, FColor::Orange, BatteryDebugString);
	}
}

bool AAeternaCharacter::RegisterScanPoint(FName ScanPointId)
{
	if (ScanPointId.IsNone() || ScannedPointIds.Contains(ScanPointId))
	{
		return false;
	}

	ScannedPointIds.Add(ScanPointId);
	const int32 CurrentCount = ScannedPointIds.Num();
	BP_ScanProgressChanged(CurrentCount, RequiredScanCount);

	UE_LOG(LogAeterna, Log, TEXT("Scan complete: %s (%d / %d)"), *ScanPointId.ToString(), CurrentCount, RequiredScanCount);
	if (GEngine)
	{
		const FString ScanMessage = FString::Printf(TEXT("Scan Complete %d / %d"), CurrentCount, RequiredScanCount);
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Cyan, ScanMessage);
	}

	if (RequiredScanCount > 0 && CurrentCount >= RequiredScanCount)
	{
		BP_AllRequiredScansCompleted();
	}

	return true;
}

bool AAeternaCharacter::HasScannedPoint(FName ScanPointId) const
{
	return !ScanPointId.IsNone() && ScannedPointIds.Contains(ScanPointId);
}

void AAeternaCharacter::UpdateHeadlampBrightness()
{
	if (!HeadlampComponent)
	{
		return;
	}

	const float BatteryAlpha = FMath::Clamp(GetBatteryNormalized(), 0.0f, 1.0f);
	const float WeightedAlpha = FMath::Pow(BatteryAlpha, BatteryBrightnessExponent);

	if (BatteryAlpha <= 0.0f)
	{
		HeadlampComponent->SetVisibility(false);
		HeadlampComponent->SetIntensity(0.0f);
		return;
	}

	HeadlampComponent->SetIntensity(FMath::Lerp(LowBatteryLightIntensity, FullBatteryLightIntensity, WeightedAlpha));
	HeadlampComponent->SetAttenuationRadius(FMath::Lerp(LowBatteryAttenuationRadius, FullBatteryAttenuationRadius, WeightedAlpha));
	HeadlampComponent->SetTemperature(FMath::Lerp(LowBatteryTemperature, FullBatteryTemperature, WeightedAlpha));
}

void AAeternaCharacter::UpdateBatteryDebugString()
{
	BatteryDebugString = FString::Printf(
		TEXT("Battery %.1f / %.1f (%.0f%%) | Headlamp %s | Drain %.1f/s"),
		CurrentBattery,
		MaxBattery,
		GetBatteryNormalized() * 100.0f,
		bHeadlampOn ? TEXT("ON") : TEXT("OFF"),
		bHeadlampOn ? HeadlampBatteryDrainPerSecond : 0.0f);
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
	return CurrentBattery;
}

float AAeternaCharacter::GetLastBatteryChargeAmount() const
{
	return LastBatteryChargeAmount;
}

FString AAeternaCharacter::GetBatteryDebugString() const
{
	return BatteryDebugString;
}

float AAeternaCharacter::GetBatteryNormalized() const
{
	return MaxBattery > 0.0f ? CurrentBattery / MaxBattery : 0.0f;
}

int32 AAeternaCharacter::GetCompletedScanCount() const
{
	return ScannedPointIds.Num();
}

AActor* AAeternaCharacter::GetFocusedInteractableActor() const
{
	return FocusedInteractableActor.Get();
}

FAeternaInteractionInfo AAeternaCharacter::GetFocusedInteractionInfo() const
{
	return FocusedInteractionInfo;
}
