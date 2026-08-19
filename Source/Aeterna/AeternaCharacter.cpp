// Copyright Epic Games, Inc. All Rights Reserved.

#include "AeternaCharacter.h"
#include "AeternaInteractableInterface.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
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
}

void AAeternaCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UpdateFocusedInteractable();
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
		BP_InteractionFailed();
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
				NewFocusedActor = HitActor;
				NewInteractionInfo = IAeternaInteractableInterface::Execute_GetInteractionInfo(HitActor, this);
			}
		}
	}

	if (FocusedInteractableActor != NewFocusedActor)
	{
		FocusedInteractableActor = NewFocusedActor;
		FocusedInteractionInfo = NewInteractionInfo;
		BP_FocusedInteractableChanged(FocusedInteractableActor.Get(), FocusedInteractionInfo);
		return;
	}

	FocusedInteractionInfo = NewInteractionInfo;
}

void AAeternaCharacter::ToggleHeadlamp()
{
	bHeadlampOn = !bHeadlampOn;
	const FString HeadlampMessage = bHeadlampOn ? TEXT("Light on") : TEXT("Light down");

	UE_LOG(LogAeterna, Log, TEXT("%s"), *HeadlampMessage);
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, HeadlampMessage);
	}

	BP_HeadlampStateChanged(bHeadlampOn);
}

bool AAeternaCharacter::IsNotebookOpen() const
{
	return bNotebookOpen;
}

bool AAeternaCharacter::IsHeadlampOn() const
{
	return bHeadlampOn;
}

AActor* AAeternaCharacter::GetFocusedInteractableActor() const
{
	return FocusedInteractableActor.Get();
}

FAeternaInteractionInfo AAeternaCharacter::GetFocusedInteractionInfo() const
{
	return FocusedInteractionInfo;
}
