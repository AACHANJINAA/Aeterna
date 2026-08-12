# AGENTS.md

이 문서는 `TPSProject` 저장소에서 작업하는 AI 에이전트의 시작 문서입니다.

## 프로젝트 개요

- 주제: 1945 와 같은 종스크롤 슈팅게임 언리얼 프로젝트
- 유형: 바이브 코딩 기반 Unreal Engine 5.8 게임 프로토타입 제작
- 주요 환경: Unreal Engine 5.8, C++, Blueprint
- 프로젝트명: `ShootingTest`
- 방식: 큐브 기반 동작 검증 후 모델과 붙이는 방식으로 단계별로 구현합니다.

## 문서 맵

- `AGENTS.md`: AI 에이전트용 시작 문서, 프로젝트 개요와 문서 맵
- `docs/WORKFLOW.md`: task 진행 순서, 구현 전 승인 절차, 상태 관리 규칙
- `docs/ARCHITECTURE.md`: 프로젝트 구조, 폴더 책임, 파일 배치 원칙
- `docs/STYLEGUIDE.md`: Unreal C++/Blueprint/에셋 작업 규칙과 파일·자산 이름 규칙
- `docs/TESTING.md`: 검증 기준과 실행 확인 원칙
- `docs/HARNESS.md`: 하네스 엔지니어링 개념 안내, 이탈 대응, 규칙 추가와 출처 기록 기준
- `docs/PLANS.md`: 단계별 진행 현황과 최근 작업 로그
- `docs/LOGS.md`: `docs/PLANS.md`에서 이동한 이전 작업 로그 보관 (WORKFLOW 절차 내부용, 별도 트리거 없음)
- `docs/PENDING_ISSUES.md`: 해소되지 않은 구조 의심과 보류 이슈 기록 (WORKFLOW 절차 내부용, 별도 트리거 없음)
- `docs/RULE_ORIGINS.md`: 규칙별 출처(원인 실패·설계 의도) 장부 (규칙 추가·수정·재검증 시에만 참조, 평소에는 읽지 않음)
- `docs/AUDIT_LOG.md`: 하네스 감사 체크리스트와 감사 이력 (사용자가 감사를 요청할 때만 참조, 평소에는 읽지 않음)
- `docs/TASK_WRITING.md`: task 문서 작성과 구현 후 갱신 규칙
- `Tasks/_template.md`: 새 task 문서 작성용 최소 복사 템플릿
- `Tasks/MAKE_REQUEST_TEMPLATE.md`: 사용자가 새 task를 요청할 때 쓰는 양식 (에이전트는 이 양식으로 된 요청을 받는 쪽)

## 먼저 읽을 문서

세션 첫 작업 시작 전 아래 문서를 순서대로 읽습니다.

1. `docs/HARNESS.md`
2. `docs/WORKFLOW.md`
3. `docs/PLANS.md`

세션 시작 시에는 위 세 문서만 읽으며, 이 문서들에서 참조하는 다른 문서는 아래 트리거가 발생하거나 현재 task의 작업 흐름을 실제로 진행할 때만 확인합니다.

같은 세션에서 이미 읽은 문서는 다시 읽지 않습니다. 재확인이 필요하면 관련 구간만 조회합니다. 다만 task 전환, 문서 충돌, 상태 불일치처럼 흐름 판단에 영향이 있는 경우에는 관련 문서를 다시 넓게 확인할 수 있습니다.

이후 같은 세션에서는 아래 상황에서 관련 문서를 기준으로 판단합니다.

- task 시작, 전환, 상태 갱신 전 → `docs/PLANS.md`, `docs/WORKFLOW.md`
- 구조 변경, 새 폴더/맵/에셋 타입 추가, 새 파일·자산의 배치 결정 전 → `docs/ARCHITECTURE.md`
- 코드 수정, 새 시스템 구현, 새 파일·자산의 이름 결정 전 → `docs/STYLEGUIDE.md`
- 검증 단계 전 → `docs/TESTING.md`
- task 문서 작성 또는 갱신 전 → `docs/TASK_WRITING.md`, `Tasks/_template.md`
- 사용자가 하네스 감사를 요청할 때 → `docs/AUDIT_LOG.md`
- 하네스 규칙 추가, 문서 절차 조정, 에이전트 흐름 이탈 대응, `.md` 문서 규칙 위반 발견 시 → 답변이나 수정 제안 전에 반드시 `docs/HARNESS.md`를 다시 확인한다.


## 문서 처리 규칙

- Markdown 문서는 UTF-8로 저장합니다.
- PowerShell에서 Markdown을 읽을 때는 `Get-Content -Encoding UTF8`을 사용합니다.
