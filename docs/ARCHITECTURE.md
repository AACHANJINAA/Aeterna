# Architecture

## 목적
이 문서는 AI 에이전트가 파일을 만들거나 이동할 때 폴더 책임, 파일 배치, Unreal 자산 배치 규칙을 확인하는 기준 문서입니다.

## 구조 원칙

- Unreal C++ 코드, Blueprint 자산, 맵, 문서 책임을 분리합니다.
- 지금 필요한 범위에서만 구조를 확장합니다.
- 파일과 자산의 이름 규칙은 `docs/STYLEGUIDE.md`의 `이름 규칙`을 따릅니다.

## 공통 폴더 책임

- `docs/`: 하네스 문서와 작업 규칙
- `Tasks/`: 개별 task 문서
- `Source/`: C++ 코드
- `Content/`: Blueprint, 맵, 애니메이션, 모델, 머티리얼 등 Unreal 자산

## 현재 프로젝트 폴더 책임과 구조

<!-- 새 프로젝트 시작 시 이 구역의 항목을 초기화한다. -->

- `Content/Maps/`: 플레이 가능한 레벨 자산
  - `L_BattleMap.umap`: 동작 검증용 기본 플레이 맵
- `Content/Blueprints/`: 플레이어 등 검증용 Blueprint 자산
  - `BP_PlayerCubeCharacter.uasset`: 현재 플레이어 조작 대상 Blueprint
  - `BP_BossCubeCharacter.uasset`: 길찾기 추적 이동 검증용 보스 Blueprint
- `Content/ParagonWraith/`: Paragon Wraith 모델, 애니메이션, 머티리얼 원본 에셋
- `Source/TPSProject/`: 게임 모듈 C++ 코드
  - `PlayerCubeCharacter.h`, `PlayerCubeCharacter.cpp`: 플레이어 입력, 카메라, 기본 사격 검증용 Character
  - `PlayerWraithAnimInstance.h`, `PlayerWraithAnimInstance.cpp`: Wraith 이동 로코모션과 조준 애니메이션 값 제공용 AnimInstance
  - `BossCubeCharacter.h`, `BossCubeCharacter.cpp`: 길찾기 추적 이동 검증용 보스 Character
  - `BossCubeAIController.h`, `BossCubeAIController.cpp`: 보스가 플레이어를 길찾기 이동 대상으로 갱신하는 AIController
  - `BossCubeAnimInstance.h`, `BossCubeAnimInstance.cpp`: 보스 FSM 상태를 AnimBP에서 읽을 값으로 제공하는 AnimInstance
  - `BossBattleArenaActor.h`, `BossBattleArenaActor.cpp`: 원형 보스 전투장의 바닥, 외곽 Mesh, 충돌 경계를 수치 기반으로 구성하는 Actor
  - `ComfyUIWorkflowRequestActor.h`, `ComfyUIWorkflowRequestActor.cpp`: Unreal Editor에서 로컬 ComfyUI workflow JSON을 `/prompt`로 요청하는 검증용 Actor


## 권장 파일 배치

- 파일은 먼저 `## 현재 프로젝트 폴더 책임과 구조`에 기록된 폴더 책임을 기준으로 배치합니다.
- 같은 성격의 파일은 기존에 정의된 책임 폴더를 우선 재사용합니다.

## 변경 원칙

- 파일이 기존에 정의된 책임 폴더로 배치할 수 없으면 새 폴더를 생성한 뒤 배치합니다.
- 새 폴더, 새 플러그인, 새 모듈, 대규모 리팩터링 같은 task 범위 밖 변경의 금지와 승인 절차는 `docs/WORKFLOW.md`의 `진행 제약`을 따릅니다.
- 새 폴더를 생성하면 `## 현재 프로젝트 폴더 책임과 구조`에 해당 폴더와 역할을 추가합니다.
- 기존 폴더에 현재 구조를 설명하는 데 중요한 파일이 추가되거나 대표 파일 구성이 바뀌면, `## 현재 프로젝트 폴더 책임과 구조`에 해당 폴더의 대표 파일 예시와 각 파일의 한 줄 역할을 실제 구조에 맞게 갱신한다.
