// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AeternaInteractableInterface.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "AeternaCharacter.generated.h"

class UInputComponent;
class USkeletalMeshComponent;
class UCameraComponent;
class UInputAction;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

/**
 *  A basic first person character
 */
UCLASS(abstract)
class AAeternaCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Pawn mesh: first person view (arms; seen only by self) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* FirstPersonMesh;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FirstPersonCameraComponent;

protected:

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, Category ="Input")
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, Category ="Input")
	UInputAction* MoveAction;

	/** Sprint Input Action */
	UPROPERTY(EditAnywhere, Category ="Input", meta = (DisplayName = "Sprint Action"))
	UInputAction* BasicSprintAction;

	/** Notebook Input Action */
	UPROPERTY(EditAnywhere, Category ="Input", meta = (DisplayName = "Notebook Action"))
	UInputAction* NotebookAction;

	/** Interact Input Action */
	UPROPERTY(EditAnywhere, Category ="Input", meta = (DisplayName = "Interact Action"))
	UInputAction* InteractAction;

	/** Headlamp Input Action */
	UPROPERTY(EditAnywhere, Category ="Input", meta = (DisplayName = "Headlamp Action"))
	UInputAction* HeadlampAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, Category ="Input")
	class UInputAction* LookAction;

	/** Mouse Look Input Action */
	UPROPERTY(EditAnywhere, Category ="Input")
	class UInputAction* MouseLookAction;

	/** 기본 걷기 속도 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement", meta = (AllowPrivateAccess = "true", DisplayName = "Walk Speed"))
	float BasicWalkSpeed = 250.0f;

	/** Shift 입력 중 사용할 달리기 속도 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement", meta = (AllowPrivateAccess = "true", DisplayName = "Sprint Speed"))
	float BasicSprintSpeed = 600.0f;

	/** 현재 달리기 입력이 유지되고 있는지 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Movement", meta = (AllowPrivateAccess = "true", DisplayName = "Sprinting"))
	bool bBasicSprinting = false;

	/** 상호작용 스캔 거리 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Interaction", meta = (AllowPrivateAccess = "true", ClampMin = "0.0", Units = "cm"))
	float InteractionTraceDistance = 300.0f;

	/** 현재 시야 중앙에 잡힌 상호작용 대상입니다. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Interaction", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<AActor> FocusedInteractableActor;

	/** 현재 상호작용 대상의 표시 정보입니다. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Interaction", meta = (AllowPrivateAccess = "true"))
	FAeternaInteractionInfo FocusedInteractionInfo;

	/** 현재 수첩이 열려 있는지 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Notebook", meta = (AllowPrivateAccess = "true"))
	bool bNotebookOpen = false;

	/** 현재 헤드램프가 켜져 있는지 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Headlamp", meta = (AllowPrivateAccess = "true"))
	bool bHeadlampOn = false;
	
public:
	AAeternaCharacter();

protected:

	/** Gameplay initialization */
	virtual void BeginPlay() override;

	/** Updates focused interaction target. */
	virtual void Tick(float DeltaSeconds) override;

	/** Called from Input Actions for movement input */
	void MoveInput(const FInputActionValue& Value);

	/** Called from Input Actions for looking input */
	void LookInput(const FInputActionValue& Value);

	/** Handles aim inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoAim(float Yaw, float Pitch);

	/** Handles move inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoMove(float Right, float Forward);

	/** Handles jump start inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpStart();

	/** Handles jump end inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpEnd();

	/** Shift 입력 시작 시 걷기에서 달리기로 전환합니다. */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void StartBasicSprint();

	/** Shift 입력 종료 시 다시 걷기 속도로 전환합니다. */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void EndBasicSprint();

	/** Tab 입력 시 수첩 열림 상태를 전환합니다. */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void ToggleNotebook();

	/** E 입력 시 시야 전방의 상호작용 대상을 호출합니다. */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void TryInteract();

	/** F 입력 시 헤드램프 상태를 전환합니다. */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void ToggleHeadlamp();

	/** 현재 시야 중앙 상호작용 대상을 갱신합니다. */
	UFUNCTION(BlueprintCallable, Category="Interaction")
	virtual void UpdateFocusedInteractable();

	/** 수첩 상태가 바뀔 때 BP에서 UI 표시를 연결합니다. */
	UFUNCTION(BlueprintImplementableEvent, Category="Notebook", meta = (DisplayName = "Notebook State Changed"))
	void BP_NotebookStateChanged(bool bOpen);

	/** 헤드램프 상태가 바뀔 때 BP에서 조명 표시를 연결합니다. */
	UFUNCTION(BlueprintImplementableEvent, Category="Headlamp", meta = (DisplayName = "Headlamp State Changed"))
	void BP_HeadlampStateChanged(bool bOn);

	/** 상호작용 대상이 없거나 상호작용할 수 없을 때 BP에서 피드백을 연결합니다. */
	UFUNCTION(BlueprintImplementableEvent, Category="Interaction", meta = (DisplayName = "Interaction Failed"))
	void BP_InteractionFailed();

	/** 조준 중인 상호작용 대상이 바뀔 때 BP에서 프롬프트 표시를 연결합니다. */
	UFUNCTION(BlueprintImplementableEvent, Category="Interaction", meta = (DisplayName = "Focused Interactable Changed"))
	void BP_FocusedInteractableChanged(AActor* NewFocusedActor, const FAeternaInteractionInfo& InteractionInfo);

protected:

	/** Set up input action bindings */
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	

public:

	/** Returns the first person mesh **/
	USkeletalMeshComponent* GetFirstPersonMesh() const { return FirstPersonMesh; }

	/** Returns first person camera component **/
	UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

	/** Returns whether the character is currently sprinting. **/
	UFUNCTION(BlueprintPure, Category="Movement")
	bool IsSprinting() const;

	/** Returns whether the notebook is currently open. **/
	UFUNCTION(BlueprintPure, Category="Notebook")
	bool IsNotebookOpen() const;

	/** Returns whether the headlamp is currently on. **/
	UFUNCTION(BlueprintPure, Category="Headlamp")
	bool IsHeadlampOn() const;

	/** Returns currently focused interactable actor. **/
	UFUNCTION(BlueprintPure, Category="Interaction")
	AActor* GetFocusedInteractableActor() const;

	/** Returns currently focused interaction info. **/
	UFUNCTION(BlueprintPure, Category="Interaction")
	FAeternaInteractionInfo GetFocusedInteractionInfo() const;

};

