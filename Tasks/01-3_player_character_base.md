# Task 01-3 - 플레이어 Character 기반 전환

## 설명
현재 플레이어 이동 검증에 사용하는 Player 구조를 `APawn` 기반에서 `ACharacter` 기반으로 전환한다. 이 단계에서는 기존 큐브 기반 이동 검증 흐름을 유지하면서, 이후 캐릭터 모델과 애니메이션을 붙일 수 있는 기본 상속 구조를 마련하는 것까지만 다룬다.

## 구현 항목
- [x] 플레이어 조작 대상이 `ACharacter`를 상속받는 구조로 전환된다.
- [x] 기존 WASD 이동 검증 흐름이 Character 기반 구조에서도 유지된다.
- [x] 실제 캐릭터 모델 없이 큐브 또는 기본 검증용 형상으로 동작을 확인할 수 있다.

## 범위 밖
- 실제 캐릭터 모델 연결
- 애니메이션 Blueprint 연결
- 카메라 기준 방향 이동
- 공격, 피격, 체력 같은 전투 로직
- 보스 로직
- 최종 플레이어 외형 연출

## 사전 전제
- 01-2 단계의 플레이어 큐브 이동 검증 코드와 입력 자산 연결 흐름이 존재한다.

## 수동 작업
- Unreal Editor에서 `APlayerCubeCharacter` 기반 Blueprint를 새로 만든다.
- 새 Blueprint에 기존 이동 Input Mapping Context와 Move Input Action을 연결한다.
- `Content/Maps/L_BattleMap.umap`에 배치된 기존 `APlayerCubePawn` 기반 Blueprint를 제거한다.
- `Content/Maps/L_BattleMap.umap`에 새 `APlayerCubeCharacter` 기반 Blueprint를 배치한다.
- 새 Blueprint의 `Auto Possess Player`가 `Player 0`인지 확인한 뒤 맵을 저장한다.

## 완료 조건

### 에이전트 확인
- [x] 관련 코드 수정 완료
- [x] 로컬 정적 점검 또는 프로젝트 구조 기준 확인 완료
- [x] Unreal C++/Blueprint/에셋 규칙 위반 없음
- [x] 현재 task 문서가 실제 구현 기준으로 갱신됨

### 결과 확인
- [x] Unreal Editor에서 `Content/Maps/L_BattleMap.umap`을 연다.
- [x] PIE를 실행한다.
- [x] 플레이어 조작 대상이 새 `APlayerCubeCharacter` 기반 Blueprint인지 확인한다.
- [x] `W`를 누르면 플레이어 큐브 Character가 한쪽 월드 방향으로 이동하는지 확인한다.
- [x] `S`를 누르면 `W`와 반대 방향으로 이동하는지 확인한다.
- [x] `A`를 누르면 플레이어 큐브 Character가 한쪽 옆 방향으로 이동하는지 확인한다.
- [x] `D`를 누르면 `A`와 반대 방향으로 이동하는지 확인한다.
- [x] 실제 캐릭터 모델 없이 큐브 형상으로 이동 검증이 되는지 확인한다.
