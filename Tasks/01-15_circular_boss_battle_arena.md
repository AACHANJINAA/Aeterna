# Task 01-15 - 원형 보스 전투장

## 설명
보스와 플레이어가 산과 절벽으로 둘러싸인 원형 전투장에서 싸우는지 검증한다. 중앙에는 넓고 평평한 전투 공간을 두고, 외곽은 산, 절벽, 지형 고저차, 또는 임시 경계 에셋으로 폐쇄형 공간처럼 보이게 구성한다. 전투장 밖으로 플레이어와 보스가 이탈하지 않도록 충돌 경계를 두며, 전투장은 수치 조정이 가능한 액터로 관리한다.

## 구현 항목
- [x] 중앙에 플레이어와 보스가 전투할 수 있는 넓고 평평한 원형 전투 공간이 있다.
- [x] 전투장 외곽이 높낮이가 다른 산, 절벽, 바위 링, 또는 임시 경계 에셋으로 둘러싸인 폐쇄형 공간처럼 보인다.
- [x] 플레이어가 전투 중 전투장 밖으로 이탈하지 못한다.
- [x] 보스가 추적, 근접 공격, 점프 내려찍기 중 전투장 밖으로 이탈하지 못한다.
- [x] 외곽 경계는 Static Mesh 기반 절벽, 바위 링, 또는 임시 Static Mesh 경계로 구성된다.
- [x] 전투장 액터에서 전투 공간 반지름, 외곽 경계 크기, 경계 높이 같은 주요 수치를 조정할 수 있다.
- [x] 전투장 구성이 기존 보스 추적, 근접 공격, 점프 내려찍기, 보스 애니메이션 상태 전환, 플레이어 이동, 카메라 조작, 조준, 사격, 점프 흐름을 깨지 않는다.

## 범위 밖
- 최종 퀄리티 산악 지형 아트 제작
- Landscape 기반 대규모 지형 제작
- 전투장 전용 조명, 안개, 날씨, 배경 연출
- 보스 신규 공격 패턴 추가
- 플레이어나 보스 스폰 연출
- 전투장 입장, 봉쇄, 클리어 연출
- 미니맵, UI, 퀘스트, 컷신

## 사전 전제
- 01-11 단계의 보스 큐브 길찾기 기반 플레이어 추적 이동이 유지되어 있다.
- 01-12 단계의 보스 근접 일반 공격 상태 전환이 유지되어 있다.
- 01-13 단계의 보스 점프 내려찍기 공격이 유지되어 있다.
- 01-14 단계의 보스 FSM 애니메이션 상태 전환이 유지되어 있다.
- `Content/Maps/L_BattleMap.umap`에서 플레이어와 보스 전투 흐름을 확인할 수 있다.

## 수동 작업
- Unreal Editor에서 C++ 변경 사항을 컴파일한다.
- `BossBattleArenaActor`를 부모 클래스로 하는 `BP_BossBattleArena` Blueprint를 만든다.
- `BP_BossBattleArena`의 중앙 바닥과 외곽 경계가 기본 Cube Static Mesh로 보이는지 확인한다.
- 필요하면 `BP_BossBattleArena`의 `Arena|Visual` 값에서 중앙 바닥용 Static Mesh와 외곽 경계용 Static Mesh를 다른 에셋으로 교체한다.
- `BP_BossBattleArena`의 `Arena|Shape` 값에서 `ArenaRadius`, `BoundarySegmentCount`, `BoundaryThickness`, `BoundaryHeight`, `BoundaryHeightVariation`, `BoundaryRadiusVariation`, `BoundaryThicknessVariation`, `BoundarySegmentOverlapScale`을 조정한다.
- `Content/Maps/L_BattleMap.umap`에 `BP_BossBattleArena`를 배치한다.
- 플레이어와 보스가 중앙 전투 공간 안에서 시작하도록 배치 위치를 확인한다.
- 전투장 배치 뒤 `NavMeshBoundsVolume`이 중앙 전투 공간을 덮도록 크기와 위치를 확인한다.

## 완료 조건

### 에이전트 확인
- [x] 관련 코드 수정 완료
- [x] 로컬 정적 점검 또는 프로젝트 구조 기준 확인 완료
- [x] Unreal C++/Blueprint/에셋 규칙 위반 없음
- [x] 현재 task 문서가 실제 구현 기준으로 갱신됨

### 결과 확인
- [x] Unreal Editor에서 `Content/Maps/L_BattleMap.umap`을 연다.
- [x] 배치된 `BP_BossBattleArena`의 중앙이 넓고 평평한 전투 공간으로 보이는지 확인한다.
- [x] 전투장 외곽이 Static Mesh 경계로 둘러싸여 높낮이가 다른 산, 절벽, 바위 링, 또는 임시 폐쇄형 공간처럼 보이는지 확인한다.
- [x] `ArenaRadius`, `BoundarySegmentCount`, `BoundaryThickness`, `BoundaryHeight`, `BoundaryHeightVariation` 값을 바꿨을 때 전투장 크기와 외곽 경계 실루엣이 갱신되는지 확인한다.
- [x] PIE를 실행한다.
- [x] 플레이어가 이동과 점프로 전투장 외곽을 넘어갈 수 없는지 확인한다.
- [x] 보스가 추적, 근접 공격, 점프 내려찍기 중 전투장 외곽 밖으로 나가지 않는지 확인한다.
- [x] 보스가 중앙 전투 공간에서 플레이어를 계속 추적할 수 있는지 확인한다.
- [x] 전투장 배치 후에도 기존 근접 공격, 점프 내려찍기, 보스 애니메이션 상태 전환, 플레이어 이동, 마우스 카메라 조작, 우클릭 조준, 좌클릭 사격, 스페이스바 점프가 기존처럼 동작하는지 확인한다.
