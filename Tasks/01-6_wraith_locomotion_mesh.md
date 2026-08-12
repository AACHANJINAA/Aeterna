# Task 01-6 - Wraith 메시와 기본 로코모션

## 설명
플레이어 큐브 표시를 제거하고 Paragon Wraith 스켈레탈 메시를 Character 기본 Mesh 컴포넌트에 연결한다. 플레이어 이동 속도와 이동 방향에 맞춰 기본 대기/이동 로코모션 애니메이션이 재생되는지 검증한다.

## 구현 항목
- [x] 플레이어 조작 대상에서 기존 큐브 표시를 제거하고 Character 기본 Mesh 컴포넌트를 사용하도록 준비한다.
- [x] Character 기본 Mesh 컴포넌트에 Paragon Wraith 스켈레탈 메시를 연결한다.
- [x] 준비된 대기, 정면 이동, 후면 이동, 좌 이동, 우 이동 애니메이션 에셋을 사용한다.
- [x] 8방향 이동 Blend Space를 구성한다.
- [x] 8방향 Blend Space에서 앞좌/앞우는 정면 애니메이션을, 뒤좌/뒤우는 후면 애니메이션을 재사용해 ±45도 방향 슬롯에 배치한다.
- [x] 이동 속도와 이동 방향 값을 제공하는 AnimInstance C++ 클래스를 구성한다.
- [x] 이동 입력 또는 이동 속도에 따라 대기와 8방향 이동 Blend Space가 재생되는 Animation Blueprint를 구성한다.
- [x] 기존 카메라, 이동 입력, 조준, 사격 동작 범위를 확장하지 않고 모델 교체와 이동 애니메이션 동작만 검증한다.

## 범위 밖
- 카메라 동작 변경
- 사격 판정 또는 사격 연출 변경
- 조준, 회피, 재장전 애니메이션
- 무기 모델 연결
- 적 모델 또는 적 AI 연결
- 최종 캐릭터 리타기팅 품질 보정
- 전투용 애니메이션 상태 머신 확장

## 사전 전제
- 01-5 단계의 플레이어 이동, 카메라, 조준, 사격 검증 구조가 유지되어 있다.
- Paragon Wraith 스켈레탈 메시 에셋이 프로젝트에 준비되어 있다.
- 대기, 정면 이동, 후면 이동, 좌 이동, 우 이동 애니메이션 에셋이 프로젝트에 준비되어 있다.

## 수동 작업
- Unreal Editor에서 `BP_PlayerCubeCharacter`를 연다.
- `Mesh (CharacterMesh0)` 컴포넌트의 Skeletal Mesh에 Paragon Wraith 메시를 연결한다.
- 대기 애니메이션으로 `Idle_NonCombat` 또는 사용할 Wraith 대기 애니메이션을 선택한다.
- 정면 이동 애니메이션으로 `Jog_Fwd` 또는 사용할 Wraith 정면 이동 애니메이션을 선택한다.
- 후면 이동 애니메이션으로 `Jog_Bwd` 또는 사용할 Wraith 후면 이동 애니메이션을 선택한다.
- 좌 이동 애니메이션으로 `Jog_Left` 또는 사용할 Wraith 좌 이동 애니메이션을 선택한다.
- 우 이동 애니메이션으로 `Jog_Right` 또는 사용할 Wraith 우 이동 애니메이션을 선택한다.
- Wraith Skeleton 기준 2D Blend Space를 새로 만든다.
- Blend Space의 방향 축은 `Direction`, 속도 축은 `Speed` 용도로 설정한다.
- Blend Space에 대기, 정면, 후면, 좌, 우 이동 애니메이션을 배치한다.
- Blend Space의 앞좌/앞우 ±45도 방향 슬롯에는 정면 이동 애니메이션을 재사용해 배치한다.
- Blend Space의 뒤좌/뒤우 ±135도 방향 슬롯에는 후면 이동 애니메이션을 재사용해 배치한다.
- `PlayerWraithAnimInstance`를 부모 클래스로 사용하는 Animation Blueprint를 만들거나 기존 Wraith Animation Blueprint의 부모 클래스를 변경한다.
- Animation Blueprint의 AnimGraph에서 `Speed`와 `Direction` 값을 Blend Space에 연결한다.
- `BP_PlayerCubeCharacter`의 `Mesh (CharacterMesh0)` 컴포넌트 Anim Class에 위 Animation Blueprint를 연결하고 저장한다.

## 완료 조건

### 에이전트 확인
- [x] 관련 코드 수정 완료
- [x] 로컬 정적 점검 또는 프로젝트 구조 기준 확인 완료
- [x] Unreal C++/Blueprint/에셋 규칙 위반 없음
- [x] 현재 task 문서가 실제 구현 기준으로 갱신됨

### 결과 확인
- [x] Unreal Editor에서 `Content/Maps/L_BattleMap.umap`을 연다.
- [x] PIE를 실행한다.
- [x] 플레이어가 큐브가 아니라 Paragon Wraith 메시로 보이는지 확인한다.
- [x] 이동 입력이 없을 때 Wraith 대기 애니메이션이 재생되는지 확인한다.
- [x] W/A/S/D와 대각선 이동 입력을 눌렀을 때 이동 방향에 맞는 Blend Space 애니메이션이 재생되는지 확인한다.
- [x] 기존 마우스 카메라 회전이 계속 동작하는지 확인한다.
- [x] 기존 좌클릭 사격과 우클릭 조준 입력이 이번 변경으로 끊기지 않았는지 확인한다.
