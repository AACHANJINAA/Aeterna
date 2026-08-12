# Task 01-2 - 플레이어 큐브 이동

## 설명
플레이어 큐브가 키보드 WASD 입력으로 실험실 안을 이동하는지 검증한다. 이 단계에서는 카메라 기준 방향 이동이 아니어도 되며, 입력에 따라 큐브가 움직이는지만 확인한다.

## 구현 항목
- [x] 플레이어 큐브가 `W`, `A`, `S`, `D` 입력에 따라 이동한다.
- [x] 이동 검증은 실제 캐릭터 모델 없이 큐브 형상으로 진행한다.

## 범위 밖
- 카메라 기준 방향 이동
- 실제 캐릭터 모델 연결
- 애니메이션 연결
- 전투 로직
- 보스 로직
- SF 실험실 분위기 연출

## 사전 전제
- `Content/Maps/L_BattleMap.umap` 레벨이 존재한다.
- `L_BattleMap`에 플레이어 시작 위치용 `PlayerStart`가 배치되어 있다.

## 수동 작업
- Unreal Editor에서 값 타입이 `Axis2D`인 Input Action을 만든다.
- Unreal Editor에서 Input Mapping Context를 만들고, 위 Input Action에 `W`, `A`, `S`, `D` 키를 매핑한다.
- `W`는 입력값이 Y 양수로 들어가도록 설정한다.
- `S`는 입력값이 Y 음수로 들어가도록 설정한다.
- `D`는 입력값이 X 양수로 들어가도록 설정한다.
- `A`는 입력값이 X 음수로 들어가도록 설정한다.
- `APlayerCubePawn` 기반 Blueprint를 만들고, 위 Input Mapping Context와 Input Action을 연결한다.
- `Content/Maps/L_BattleMap.umap`에 위 Blueprint를 배치하고 `Auto Possess Player`가 `Player 0`인지 확인한 뒤 저장한다.

## 완료 조건

### 에이전트 확인
- [x] 관련 코드 수정 완료
- [x] 로컬 정적 점검 또는 프로젝트 구조 기준 확인 완료
- [x] Unreal C++/Blueprint/에셋 규칙 위반 없음
- [x] 현재 task 문서가 실제 구현 기준으로 갱신됨

### 결과 확인
- [x] Unreal Editor에서 `Content/Maps/L_BattleMap.umap`을 연다.
- [x] PIE를 실행한다.
- [x] `W`를 누르면 플레이어 큐브가 한쪽 월드 방향으로 이동하는지 확인한다.
- [x] `S`를 누르면 `W`와 반대 방향으로 이동하는지 확인한다.
- [x] `A`를 누르면 플레이어 큐브가 한쪽 옆 방향으로 이동하는지 확인한다.
- [x] `D`를 누르면 `A`와 반대 방향으로 이동하는지 확인한다.
- [x] 실제 캐릭터 모델 없이 큐브 형상으로 이동 검증이 되는지 확인한다.
