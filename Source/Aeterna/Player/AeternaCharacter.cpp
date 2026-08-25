// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/AeternaCharacter.h"

#include "Aeterna.h"
#include "Interaction/AeternaInteractableActor.h"
#include "Interaction/AeternaInteractableInterface.h"
#include "Player/Components/AeternaBatteryComponent.h"
#include "Player/Components/AeternaBatteryHudComponent.h"
#include "Player/Components/AeternaClockComponent.h"
#include "Player/Components/AeternaHeadBobComponent.h"
#include "Player/Components/AeternaInteractionComponent.h"
#include "Player/Components/AeternaInteractionPromptComponent.h"
#include "Player/Components/AeternaObjectiveHudComponent.h"
#include "Player/Components/AeternaScanProgressComponent.h"
#include "Player/Components/AeternaCarryComponent.h"
#include "Player/Components/AeternaGazeRuleComponent.h"
#include "Player/Components/AeternaVanishRuleComponent.h"
#include "Player/Components/AeternaClockFreezeRuleComponent.h"
#include "Player/Components/AeternaCameraFallComponent.h"

#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SpotLightComponent.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "InputActionValue.h"
#include "InputCoreTypes.h"

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
	BatteryHudComponent = CreateDefaultSubobject<UAeternaBatteryHudComponent>(TEXT("BatteryHudComponent"));
	ClockComponent = CreateDefaultSubobject<UAeternaClockComponent>(TEXT("ClockComponent"));
	InteractionComponent = CreateDefaultSubobject<UAeternaInteractionComponent>(TEXT("InteractionComponent"));
	InteractionPromptComponent = CreateDefaultSubobject<UAeternaInteractionPromptComponent>(TEXT("InteractionPromptComponent"));
	ObjectiveHudComponent = CreateDefaultSubobject<UAeternaObjectiveHudComponent>(TEXT("ObjectiveHudComponent"));
	ScanProgressComponent = CreateDefaultSubobject<UAeternaScanProgressComponent>(TEXT("ScanProgressComponent"));
	CarryComponent = CreateDefaultSubobject<UAeternaCarryComponent>(TEXT("CarryComponent"));
	GazeRuleComponent = CreateDefaultSubobject<UAeternaGazeRuleComponent>(TEXT("GazeRuleComponent"));
	VanishRuleComponent = CreateDefaultSubobject<UAeternaVanishRuleComponent>(TEXT("VanishRuleComponent"));
	ClockFreezeRuleComponent = CreateDefaultSubobject<UAeternaClockFreezeRuleComponent>(TEXT("ClockFreezeRuleComponent"));
	CameraFallComponent = CreateDefaultSubobject<UAeternaCameraFallComponent>(TEXT("CameraFallComponent"));

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
		if (BatteryHudComponent)
		{
			BatteryHudComponent->InitializePlayerComponent(this);
			BatteryHudComponent->UpdateBatteryHud(BatteryComponent->GetCurrentBattery(), BatteryComponent->GetMaxBattery(), BatteryComponent->GetBatteryNormalized());
		}
		BP_BatteryChanged(BatteryComponent->GetCurrentBattery(), BatteryComponent->GetMaxBattery(), BatteryComponent->GetBatteryNormalized());
	}
	if (ClockComponent)
	{
		ClockComponent->InitializePlayerComponent(this);
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
	// BP가 이 컴포넌트보다 먼저 컴파일됐으면 인스턴싱이 갱신되지 않아,
	// 인스턴스가 CDO 아키타입의 컴포넌트를 그대로 가리킬 수 있습니다.
	// 그 컴포넌트는 월드에 속하지 않아 트레이스가 불가능하므로 여기서 갈아끼웁니다.
	if (!CarryComponent || CarryComponent->GetOwner() != this)
	{
		const TCHAR* FallbackReason = CarryComponent ? TEXT("아키타입 컴포넌트를 가리킴") : TEXT("컴포넌트 없음");

		CarryComponent = NewObject<UAeternaCarryComponent>(this, UAeternaCarryComponent::StaticClass(), TEXT("CarryComponentRuntime"));
		CarryComponent->RegisterComponent();

		UE_LOG(LogAeterna, Warning,
			TEXT("[Carry] CarryComponent를 런타임에 생성했습니다 (%s). BP_Player를 열어 Compile 후 Save 하십시오."),
			FallbackReason);
	}

	if (CarryComponent)
	{
		CarryComponent->InitializePlayerComponent(this);
	}

	if (!GazeRuleComponent || GazeRuleComponent->GetOwner() != this)
	{
		GazeRuleComponent = NewObject<UAeternaGazeRuleComponent>(this, UAeternaGazeRuleComponent::StaticClass(), TEXT("GazeRuleComponentRuntime"));
		GazeRuleComponent->RegisterComponent();
		UE_LOG(LogAeterna, Warning, TEXT("[Gaze] GazeRuleComponent를 런타임에 생성했습니다. BP_Player를 열어 Compile 후 Save 하십시오."));
	}

	GazeRuleComponent->InitializePlayerComponent(this);

	if (!VanishRuleComponent || VanishRuleComponent->GetOwner() != this)
	{
		VanishRuleComponent = NewObject<UAeternaVanishRuleComponent>(this, UAeternaVanishRuleComponent::StaticClass(), TEXT("VanishRuleComponentRuntime"));
		VanishRuleComponent->RegisterComponent();
		UE_LOG(LogAeterna, Warning, TEXT("[Vanish] VanishRuleComponent를 런타임에 생성했습니다. BP_Player를 열어 Compile 후 Save 하십시오."));
	}

	VanishRuleComponent->InitializePlayerComponent(this);

	// 낙하 연출은 규칙들이 공용하므로 규칙 컴포넌트보다 먼저 준비돼 있어야 합니다.
	if (!CameraFallComponent || CameraFallComponent->GetOwner() != this)
	{
		CameraFallComponent = NewObject<UAeternaCameraFallComponent>(this, UAeternaCameraFallComponent::StaticClass(), TEXT("CameraFallComponentRuntime"));
		CameraFallComponent->RegisterComponent();
		UE_LOG(LogAeterna, Warning, TEXT("[CameraFall] 런타임에 생성했습니다. BP_Player를 열어 Compile 후 Save 하십시오."));
	}

	CameraFallComponent->InitializePlayerComponent(this);

	if (!ClockFreezeRuleComponent || ClockFreezeRuleComponent->GetOwner() != this)
	{
		ClockFreezeRuleComponent = NewObject<UAeternaClockFreezeRuleComponent>(this, UAeternaClockFreezeRuleComponent::StaticClass(), TEXT("ClockFreezeRuleComponentRuntime"));
		ClockFreezeRuleComponent->RegisterComponent();
		UE_LOG(LogAeterna, Warning, TEXT("[ClockFreeze] 런타임에 생성했습니다. BP_Player를 열어 Compile 후 Save 하십시오."));
	}

	ClockFreezeRuleComponent->InitializePlayerComponent(this);

	if (ScanProgressComponent)
	{
		ScanProgressComponent->InitializePlayerComponent(this);
	}

	if (!ObjectiveHudComponent || ObjectiveHudComponent->GetOwner() != this)
	{
		ObjectiveHudComponent = NewObject<UAeternaObjectiveHudComponent>(this, UAeternaObjectiveHudComponent::StaticClass(), TEXT("ObjectiveHudComponentRuntime"));
		ObjectiveHudComponent->RegisterComponent();
		UE_LOG(LogAeterna, Warning, TEXT("[ObjectiveHUD] 런타임에 생성했습니다. BP_Player를 열어 Compile 후 Save 하십시오."));
	}

	ObjectiveHudComponent->InitializePlayerComponent(this);
	ObjectiveHudComponent->RefreshObjectiveHud();
}

void AAeternaCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UpdateHeadBob(DeltaSeconds);
	UpdateFocusedInteractable();

	if (BatteryComponent && BatteryComponent->TickBattery(DeltaSeconds, bHeadlampOn))
	{
		if (BatteryHudComponent)
		{
			BatteryHudComponent->UpdateBatteryHud(BatteryComponent->GetCurrentBattery(), BatteryComponent->GetMaxBattery(), BatteryComponent->GetBatteryNormalized());
		}

		BP_BatteryChanged(BatteryComponent->GetCurrentBattery(), BatteryComponent->GetMaxBattery(), BatteryComponent->GetBatteryNormalized());

		if (bHeadlampOn && BatteryComponent->GetCurrentBattery() <= 0.0f)
		{
			bHeadlampOn = false;
			if (HeadlampComponent)
			{
				HeadlampComponent->SetVisibility(false);
			}
			BP_HeadlampStateChanged(false);
		}
	}
}

void AAeternaCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{	
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Jumping is kept for future experiments, but disabled by default for Aeterna's fixed input set.
		if (bEnableJumpInput && JumpAction)
		{
			EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AAeternaCharacter::DoJumpStart);
			EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &AAeternaCharacter::DoJumpEnd);
		}

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
			EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Completed, this, &AAeternaCharacter::EndInteract);
			EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Canceled, this, &AAeternaCharacter::EndInteract);
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
		UE_LOG(LogAeterna, Error, TEXT("'%s' requires an Enhanced Input Component."), *GetNameSafe(this));
	}

#if !UE_BUILD_SHIPPING
	PlayerInputComponent->BindKey(EKeys::P, IE_Pressed, this, &AAeternaCharacter::AdvanceDebugClock);
#endif
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
	if (!bEnableJumpInput)
	{
		return;
	}

	// pass Jump to the character
	Jump();
}

void AAeternaCharacter::DoJumpEnd()
{
	if (!bEnableJumpInput)
	{
		return;
	}

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
	BP_NotebookStateChanged(bNotebookOpen);
}

void AAeternaCharacter::TryInteract()
{
	// 운반 대상을 조준 중이면 상호작용보다 먼저 집습니다. E를 누르고 있는 동안 들려 있습니다.
	if (CarryComponent && CarryComponent->TryStartCarry())
	{
		return;
	}

	if (!InteractionComponent)
	{
		BP_InteractionFailed();
		return;
	}

	const bool bInteractionSucceeded = InteractionComponent->TryInteract(this);
	if (bInteractionSucceeded)
	{
		if (InteractionPromptComponent)
		{
			InteractionPromptComponent->ShowSuccessFeedback(InteractionComponent->GetFocusedInteractableActor());
		}
		return;
	}
	BP_InteractionFailed();
}

void AAeternaCharacter::EndInteract()
{
	if (CarryComponent)
	{
		CarryComponent->StopCarry();
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
	SetHeadlampOn(!bHeadlampOn);
}

void AAeternaCharacter::SetHeadlampOn(bool bOn)
{
	// 끄는 것은 언제나 되지만, 켜는 것은 배터리가 남아 있어야 합니다.
	const bool bTargetOn = bOn && BatteryComponent && BatteryComponent->GetCurrentBattery() > 0.0f;
	const bool bStateChanged = (bHeadlampOn != bTargetOn);

	bHeadlampOn = bTargetOn;

	// 상태가 그대로여도 조명 가시성은 매번 다시 맞춥니다.
	// 부모 메시의 SetVisibility가 자식까지 전파되면서 램프만 따로 켜져 있을 수 있습니다.
	if (HeadlampComponent)
	{
		HeadlampComponent->SetVisibility(bHeadlampOn);
	}
	UpdateHeadlampBrightness();

	if (bStateChanged)
	{
		BP_HeadlampStateChanged(bHeadlampOn);
	}
}

void AAeternaCharacter::AddPlayerBattery(float Amount)
{
	if (!BatteryComponent || Amount <= 0.0f)
	{
		return;
	}

	BatteryComponent->AddBattery(Amount);
	BatteryComponent->UpdateBatteryDebugString(bHeadlampOn);
	if (BatteryHudComponent)
	{
		BatteryHudComponent->UpdateBatteryHud(BatteryComponent->GetCurrentBattery(), BatteryComponent->GetMaxBattery(), BatteryComponent->GetBatteryNormalized());
	}
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

bool AAeternaCharacter::HasCompletedRequiredScans() const
{
	return ScanProgressComponent && ScanProgressComponent->HasCompletedRequiredScans();
}

void AAeternaCharacter::SetRequiredScanCount(int32 RequiredCount)
{
	if (ScanProgressComponent)
	{
		ScanProgressComponent->SetRequiredScanCount(RequiredCount);
		BP_ScanProgressChanged(ScanProgressComponent->GetCompletedScanCount(), ScanProgressComponent->GetRequiredScanCount());
	}
}

void AAeternaCharacter::ResetScanProgress()
{
	if (ScanProgressComponent)
	{
		ScanProgressComponent->ResetScanProgress();
		BP_ScanProgressChanged(ScanProgressComponent->GetCompletedScanCount(), ScanProgressComponent->GetRequiredScanCount());
	}
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

void AAeternaCharacter::AdvanceDebugClock()
{
	if (ClockComponent)
	{
		ClockComponent->AdvanceDebugClockStep();
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

int32 AAeternaCharacter::GetClockMinutes() const
{
	return ClockComponent ? ClockComponent->GetClockMinutes() : 0;
}

int32 AAeternaCharacter::GetCompletedScanCount() const
{
	return ScanProgressComponent ? ScanProgressComponent->GetCompletedScanCount() : 0;
}

int32 AAeternaCharacter::GetRequiredScanCount() const
{
	return ScanProgressComponent ? ScanProgressComponent->GetRequiredScanCount() : 0;
}

AActor* AAeternaCharacter::GetFocusedInteractableActor() const
{
	return InteractionComponent ? InteractionComponent->GetFocusedInteractableActor() : nullptr;
}

FAeternaInteractionInfo AAeternaCharacter::GetFocusedInteractionInfo() const
{
	return InteractionComponent ? InteractionComponent->GetFocusedInteractionInfo() : FAeternaInteractionInfo();
}

bool AAeternaCharacter::IsCarrying() const
{
	return CarryComponent && CarryComponent->IsCarrying();
}

AActor* AAeternaCharacter::GetCarriedActor() const
{
	return CarryComponent ? CarryComponent->GetCarriedActor() : nullptr;
}
