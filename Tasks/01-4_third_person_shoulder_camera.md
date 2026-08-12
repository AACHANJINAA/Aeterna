# Task 01-4 - 3인칭 숄더뷰 카메라

## 설명
플레이어 Character에 일반적인 3인칭 숄더뷰 카메라를 붙이고, 마우스 입력으로 시점을 회전할 수 있게 한다. WASD 이동은 월드 고정 방향이 아니라 현재 카메라가 바라보는 방향을 기준으로 동작해야 한다.

## 구현 항목
- [x] 플레이어를 따라가는 3인칭 숄더뷰 카메라가 동작한다.
- [x] 마우스 입력으로 카메라 시점을 좌우와 상하로 회전할 수 있다.
- [x] WASD 이동이 카메라 기준 방향과 연동된다.
- [x] 실제 캐릭터 모델 없이 큐브 또는 기본 검증용 형상으로 카메라와 이동을 확인할 수 있다.

## 범위 밖
- 실제 캐릭터 모델 연결
- 애니메이션 Blueprint 연결
- 조준, 발사, 피격 같은 전투 로직
- 카메라 충돌 보정의 세부 튜닝
- 최종 카메라 연출

## 사전 전제
- 01-3 단계의 `APlayerCubeCharacter` 기반 플레이어 구조가 존재한다.
- 기존 WASD 이동 Input Action과 Input Mapping Context 연결 흐름이 존재한다.

## 수동 작업
- Unreal Editor에서 값 타입이 `Axis2D`인 마우스 시점 회전용 Input Action을 만든다.
- 기존 Input Mapping Context에 위 Input Action을 추가하고 마우스 X/Y 입력을 매핑한다.
- `APlayerCubeCharacter` 기반 Blueprint에서 `LookAction`에 위 Input Action을 연결한다.
- 기존 이동 Input Mapping Context와 Move Input Action 연결이 유지되어 있는지 확인한다.
- `Content/Maps/L_BattleMap.umap`에 배치된 `APlayerCubeCharacter` 기반 Blueprint를 저장한다.

## 완료 조건

### 에이전트 확인
- [x] 관련 코드 수정 완료
- [x] 로컬 정적 점검 또는 프로젝트 구조 기준 확인 완료
- [x] Unreal C++/Blueprint/에셋 규칙 위반 없음
- [x] 현재 task 문서가 실제 구현 기준으로 갱신됨

### 결과 확인
- [x] Unreal Editor에서 `Content/Maps/L_BattleMap.umap`을 연다.
- [x] PIE를 실행한다.
- [x] 플레이어 뒤쪽 어깨 너머 위치에서 카메라가 따라오는지 확인한다.
- [x] 마우스를 좌우로 움직이면 카메라 시점이 좌우로 회전하는지 확인한다.
- [x] 마우스를 위아래로 움직이면 카메라 시점이 상하로 회전하는지 확인한다.
- [x] 카메라 방향을 바꾼 뒤 `W`를 누르면 현재 카메라가 바라보는 수평 방향으로 이동하는지 확인한다.
- [x] 카메라 방향을 바꾼 뒤 `A`, `S`, `D` 입력도 카메라 기준 방향에 맞게 이동하는지 확인한다.
- [x] 실제 캐릭터 모델 없이 큐브 형상으로 카메라와 이동 검증이 되는지 확인한다.
