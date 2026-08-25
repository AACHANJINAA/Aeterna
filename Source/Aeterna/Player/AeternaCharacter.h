// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interaction/AeternaInteractableInterface.h"
#include "Logging/LogMacros.h"
#include "AeternaCharacter.generated.h"

class UInputComponent;
class USkeletalMeshComponent;
class UCameraComponent;
class UInputAction;
class USpotLightComponent;

class UAeternaBatteryComponent;
class UAeternaBatteryHudComponent;
class UAeternaClockComponent;
class UAeternaHeadBobComponent;
class UAeternaInteractionComponent;
class UAeternaInteractionPromptComponent;
class UAeternaScanProgressComponent;
class UAeternaCarryComponent;
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

	/** M-05 머리 위 내장 헤드램프 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USpotLightComponent* HeadlampComponent;

	/** 1인칭 카메라 흔들림 처리 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UAeternaHeadBobComponent* HeadBobComponent;

	/** 플레이어 배터리와 헤드램프 밝기 처리 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UAeternaBatteryComponent* BatteryComponent;

	/** 플레이어 배터리 HUD 표시 처리 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UAeternaBatteryHudComponent* BatteryHudComponent;

	/** 화면 시계와 테스트용 시간 진행 처리 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UAeternaClockComponent* ClockComponent;

	/** 상호작용 라인트레이스와 실행 처리 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UAeternaInteractionComponent* InteractionComponent;

	/** 상호작용 프롬프트 위젯 표시 처리 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UAeternaInteractionPromptComponent* InteractionPromptComponent;

	/** 스캔 진행도 처리 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UAeternaScanProgressComponent* ScanProgressComponent;

	/** E 홀드 운반과 제자리 설치 처리 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UAeternaCarryComponent* CarryComponent;

protected:

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, Category ="Input")
	UInputAction* JumpAction;

	/** 향후 필요할 때만 켜는 점프 입력입니다. Aeterna 기본 조작에서는 꺼둡니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category ="Input", meta=(DisplayName = "Enable Jump Input"))
	bool bEnableJumpInput = false;

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

	/** 현재 수첩이 열려 있는지 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Notebook", meta = (AllowPrivateAccess = "true"))
	bool bNotebookOpen = false;

	/** 현재 헤드램프가 켜져 있는지 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Headlamp", meta = (AllowPrivateAccess = "true"))
	bool bHeadlampOn = false;
	
public:
	AAeternaCharacter();

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

	/** E 입력을 뗄 때 들고 있던 물체를 놓습니다. */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void EndInteract();

	/** F 입력 시 헤드램프 상태를 전환합니다. */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void ToggleHeadlamp();

	/** 속도 상태에 맞춰 1인칭 카메라 헤드밥을 갱신합니다. */
	UFUNCTION(BlueprintCallable, Category="Camera|Head Bob")
	virtual void UpdateHeadBob(float DeltaSeconds);

	/** 현재 시야 중앙 상호작용 대상을 갱신합니다. */
	UFUNCTION(BlueprintCallable, Category="Interaction")
	virtual void UpdateFocusedInteractable();

	/** 플레이어 배터리를 충전합니다. */
	UFUNCTION(BlueprintCallable, Category="Battery")
	virtual void AddPlayerBattery(float Amount);

	/** 스캔 지점 완료를 기록합니다. */
	UFUNCTION(BlueprintCallable, Category="Scan")
	virtual bool RegisterScanPoint(FName ScanPointId);

	/** 이미 기록된 스캔 지점인지 확인합니다. */
	UFUNCTION(BlueprintPure, Category="Scan")
	virtual bool HasScannedPoint(FName ScanPointId) const;

	/** 필수 스캔 개수를 모두 채웠는지 확인합니다. */
	UFUNCTION(BlueprintPure, Category="Scan")
	virtual bool HasCompletedRequiredScans() const;

	/** 시나리오나 테스트 BP에서 필요한 스캔 개수를 설정합니다. */
	UFUNCTION(BlueprintCallable, Category="Scan")
	virtual void SetRequiredScanCount(int32 RequiredCount);

	/** 시나리오 재시작이나 테스트용으로 스캔 진행도를 초기화합니다. */
	UFUNCTION(BlueprintCallable, Category="Scan")
	virtual void ResetScanProgress();

	/** 헤드램프 조명 값을 현재 배터리 잔량에 맞춰 갱신합니다. */
	UFUNCTION(BlueprintCallable, Category="Headlamp")
	virtual void UpdateHeadlampBrightness();

	/** 배터리 디버그 문자열을 현재 값으로 갱신합니다. */
	UFUNCTION(BlueprintCallable, Category="Battery|Debug")
	virtual void UpdateBatteryDebugString();

	/** 개발 테스트용으로 화면 시계를 30분 진행합니다. */
	UFUNCTION(BlueprintCallable, Category="Clock|Debug")
	virtual void AdvanceDebugClock();

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

	/** 배터리 잔량이 바뀔 때 BP에서 게이지 표시를 연결합니다. */
	UFUNCTION(BlueprintImplementableEvent, Category="Battery", meta = (DisplayName = "Battery Changed"))
	void BP_BatteryChanged(float Current, float Max, float Normalized);

	/** 스캔 진행도가 바뀔 때 BP에서 표시를 연결합니다. */
	UFUNCTION(BlueprintImplementableEvent, Category="Scan", meta = (DisplayName = "Scan Progress Changed"))
	void BP_ScanProgressChanged(int32 CurrentCount, int32 RequiredCount);

	/** 필수 스캔 지점이 모두 완료됐을 때 BP에서 목표 완료 연출을 연결합니다. */
	UFUNCTION(BlueprintImplementableEvent, Category="Scan", meta = (DisplayName = "All Required Scans Completed"))
	void BP_AllRequiredScansCompleted();

	/** 물체를 들기 시작할 때 BP에서 연출을 연결합니다. */
	UFUNCTION(BlueprintImplementableEvent, Category="Carry", meta = (DisplayName = "Carry Started"))
	void BP_CarryStarted(AActor* CarriedActor, FName CarryId);

	/** 물체를 제자리가 아닌 곳에 놓을 때 BP에서 연출을 연결합니다. */
	UFUNCTION(BlueprintImplementableEvent, Category="Carry", meta = (DisplayName = "Carry Stopped"))
	void BP_CarryStopped(AActor* DroppedActor, FName CarryId);

	/** 물체가 제자리에 설치될 때 BP에서 연출을 연결합니다. */
	UFUNCTION(BlueprintImplementableEvent, Category="Carry", meta = (DisplayName = "Carry Installed"))
	void BP_CarryInstalled(AActor* InstalledActor, FName CarryId);

	/** 조준 중인 운반 후보가 바뀔 때 BP에서 프롬프트 표시를 연결합니다. */
	UFUNCTION(BlueprintImplementableEvent, Category="Carry", meta = (DisplayName = "Carry Target Changed"))
	void BP_CarryTargetChanged(AActor* NewTargetActor, FName CarryId);

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

	/** Returns current player battery. **/
	UFUNCTION(BlueprintPure, Category="Battery")
	float GetCurrentBattery() const;

	/** Returns last charged battery amount. **/
	UFUNCTION(BlueprintPure, Category="Battery")
	float GetLastBatteryChargeAmount() const;

	/** Returns battery debug string. **/
	UFUNCTION(BlueprintPure, Category="Battery|Debug")
	FString GetBatteryDebugString() const;

	/** Returns normalized player battery from 0 to 1. **/
	UFUNCTION(BlueprintPure, Category="Battery")
	float GetBatteryNormalized() const;

	/** Returns current clock minutes from 00:00. **/
	UFUNCTION(BlueprintPure, Category="Clock")
	int32 GetClockMinutes() const;

	/** Returns completed scan count. **/
	UFUNCTION(BlueprintPure, Category="Scan")
	int32 GetCompletedScanCount() const;

	/** Returns required scan count. **/
	UFUNCTION(BlueprintPure, Category="Scan")
	int32 GetRequiredScanCount() const;

	/** Returns currently focused interactable actor. **/
	UFUNCTION(BlueprintPure, Category="Interaction")
	AActor* GetFocusedInteractableActor() const;

	/** Returns currently focused interaction info. **/
	UFUNCTION(BlueprintPure, Category="Interaction")
	FAeternaInteractionInfo GetFocusedInteractionInfo() const;

	/** Returns whether the player is currently carrying an object. **/
	UFUNCTION(BlueprintPure, Category="Carry")
	bool IsCarrying() const;

	/** Returns the actor currently being carried. **/
	UFUNCTION(BlueprintPure, Category="Carry")
	AActor* GetCarriedActor() const;

};
