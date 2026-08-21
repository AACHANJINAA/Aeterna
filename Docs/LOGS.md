# Logs

`Docs/PLANS.md`의 최근 작업 로그에서 이동한 이전 작업 로그를 보관하는 문서입니다.
최신 진행 상황은 `Docs/PLANS.md`를 먼저 확인합니다.

<!-- 새 프로젝트 시작 시 이전 프로젝트의 보관 로그를 초기화한다. -->

## 보관 로그

- 2026-08-12: Aeterna용 하네스 문서 초기화. 8개 30분 시나리오, 경비실 시작·종료, 현재 시나리오 롤백, 수첩 중심 규칙 전달 기준을 초기 계획에 반영. 구현은 시작하지 않음.
- 2026-08-14: GDD v1.3을 단일 원본으로 전체 문서 정합화. 8개 30분 슬롯 → **밤 3개(S01~S03), 각 밤 01:00~05:00 전체**로 교체. 클리어 시 경비실 복귀 → **완료 지점 페이드아웃 →「Day N」카드**로 교체. 고정 담당제(GDD §9) 채택, 레벨 네이밍 `Museum_Persistent`/`Sub_*` 통일, `Docs/` 경로 대소문자 정정. 작업리스트를 새 구조로 갱신. 구현은 시작하지 않음.
- 2026-08-14: GDD §3-2에 **잔량-밝기 연동** 확정 반영 (강도·도달 거리·색온도 연속 감쇠). §10 #3 튜닝 항목에 감쇠 곡선 추가, #10을 하강 나선 리스크로 확장·우선순위 상향. 단계 01-5 추가.

## CJ 개발 로그

### 2026-08-19

- Player
	- Input
		- WASD 걷기 이동 기준 정리
		- Shift 달리기 전환 추가
		- Tab 수첩 열림/닫힘 토글 추가
		- E 상호작용 입력 연결
		- F 헤드램프 ON/OFF 토글 추가
	- Scan
		- 상호작용 Interface 기반 Scan 처리 추가
		- Scan 진행도 기록 컴포넌트 추가
		- 동일 Scan ID 중복 등록 방지 추가
		- 기본 프롬프트 문구를 `Scan Exhibit`로 변경
	- Charge
		- 배터리 충전 상호작용 추가
		- 충전량 변수화
		- 충전 완료 로그 유지
	- HeadLamp
		- 헤드램프 배터리 소모 추가
		- 배터리 잔량 기반 밝기/거리/색온도 감쇠 추가
		- 헤드램프 색상 조절 변수 추가
		- 헤드램프 빔 너비 조절 변수 추가
	- Interaction UI
		- 상호작용 대상 상단 추적 프롬프트 추가
		- 포커스 해제 후 1초 딜레이 숨김 추가
		- 상호작용 성공 시 `Success` 0.7초 표시 추가
		- `Success` 표시 상태 bool 추가
		- `Success` 텍스트 색상 조절 변수 추가
	- Camera
		- Idle/Walk/Sprint Head Bob 추가
	- Refactor
		- 플레이어 기능을 Component 단위로 분리
		- `AeternaPlayerComponent` 공통 부모 추가
		- Source 폴더를 `Core`, `Interaction`, `Player`, `Player/Components`로 정리
		- include 경로를 기능 폴더 기준으로 정리

- Git
	- 외부 에셋/플러그인 폴더 `.gitignore` 제외 처리
	- C++ 화면 디버그 메시지 제거 후 `UE_LOG` 중심으로 정리

### 2026-08-21

- Clock / Scenario Time Loop
	- `UGameClockSubsystem` 기반 기본 시간 루프 정리
	- 기본 게임 시간 범위를 01:00~05:00으로 설정
	- 기본 시간 배속을 게임 30분 = 실제 30초 기준으로 조정
	- 분 변경, 시각 이벤트 도달, 종료 브로드캐스트 추가
	- 시계 HUD가 GameClock 분 변경 이벤트를 구독하도록 연결
	- 전체화면/창 크기 변경 시 시계 HUD가 현재 뷰포트 중앙 상단을 다시 계산하도록 수정

- Player / Interaction
	- 점프 입력 로직은 유지하되 기본 비활성화
	- `bEnableJumpInput`이 켜진 경우에만 JumpAction 바인딩
	- 스캔 목표 개수를 Blueprint에서 설정할 수 있도록 `SetRequiredScanCount` 추가

- Performance / Cleanup
	- 배터리 HUD 매 Tick 갱신 제거
	- 배터리 값 변경 시에만 HUD와 BP 이벤트를 갱신하도록 변경
	- 배터리 HUD 내부에서 동일 값 반복 갱신을 건너뛰도록 캐시 비교 추가
	- 배터리 디버그 로그 옵션과 반복 로그 타이머 제거
	- 일반 상태 확인용 `UE_LOG` 정리, 오류성 로그만 유지

- Architecture Notes
	- 현재 컴포넌트 분리는 기능 단위로 양호하나, `AAeternaCharacter`가 여러 컴포넌트의 흐름을 직접 조율하는 경향이 있음
	- 추후 `ScenarioManager` 추가 시 시계, 배터리, 스캔, 현재 밤 초기화 흐름을 Character 중심에서 ScenarioManager/상태 관리 책임으로 이동할 필요 있음
