# Task 01-16 - 로컬 ComfyUI workflow 요청 검증

## 설명
Unreal Editor에서 로컬 ComfyUI 서버로 workflow 요청을 보내고 응답을 확인할 수 있는지 검증한다. ComfyUI 서버는 로컬에서 이미 실행 중인 것으로 전제하며, 기본 서버 주소는 `http://.127.0.0.1:8188`로 두되 이후 변경 가능한 설정값으로 관리한다. 준비된 workflow JSON 파일을 읽고 필요한 프롬프트와 이미지 크기 값을 덮어쓴 뒤 ComfyUI의 `/prompt` 엔드포인트로 요청을 보낼 수 있어야 한다.

## 구현 항목
- [x] Unreal Editor에서 실행할 수 있는 방식으로 로컬 ComfyUI 서버 연결을 검증할 수 있다.
- [x] ComfyUI 서버 주소는 기본값 `http://.127.0.0.1:8188`을 가지며 나중에 변경 가능한 설정값으로 둔다.
- [x] 준비된 workflow JSON 파일을 읽어 요청 본문으로 사용할 수 있다.
- [x] 읽은 workflow JSON을 ComfyUI `/prompt` API가 기대하는 `prompt` 본문으로 변환해 요청할 수 있다.
- [x] 저장된 ComfyUI UI workflow JSON에서 프롬프트 내용과 이미지 크기 값을 덮어쓸 수 있다.
- [x] `/prompt` 요청 결과의 성공 또는 실패 응답을 Unreal Editor 안에서 확인할 수 있다.
- [x] ComfyUI 서버가 실행 중이지 않거나 요청에 실패했을 때 실패 사유를 확인할 수 있다.

## 범위 밖
- ComfyUI 설치 또는 실행 자동화
- workflow JSON 생성, 편집, 검증 도구 제작
- 이미지 생성 결과물 다운로드, 저장, 임포트
- Unreal 에셋 자동 생성 파이프라인
- 게임플레이 기능, 보스 전투, 플레이어 조작 변경
- 원격 ComfyUI 서버 인증, 계정, 토큰 관리

## 사전 전제
- 01-15 단계까지의 기본 맵과 보스 전투 검증 흐름이 유지되어 있다.
- ComfyUI 서버는 로컬 PC에서 별도로 실행되어 있다.
- 요청에 사용할 ComfyUI workflow JSON 파일은 미리 준비되어 있다.

## 수동 작업
- Unreal Editor에서 C++ 변경 사항을 컴파일한다.
- `ComfyUIWorkflowRequestActor`를 부모 클래스로 하는 Blueprint를 만든다.
- 만든 Blueprint를 Editor에서 열거나 레벨에 배치한 뒤 선택한다.
- Details 패널의 `ComfyUI|Request` 값에서 `Workflow Json File`에 준비된 ComfyUI workflow JSON 파일을 지정한다.
- 기본 서버 주소를 그대로 사용할 수 없는 경우 `Server Base Url` 값을 실제 로컬 ComfyUI 주소로 바꾼다.
- `/prompt` 요청이 오래 걸리면 `Request Timeout Seconds` 값을 조정한다.
- `ComfyUI|Override` 값에서 바꿀 긍정 프롬프트, 부정 프롬프트, 이미지 너비, 이미지 높이를 입력한다.
- 자동 감지한 노드가 원하는 노드와 다르면 `ComfyUI|Override|Advanced` 값에서 긍정 프롬프트 노드 ID, 부정 프롬프트 노드 ID, 이미지 크기 노드 ID를 직접 입력한다.

## 완료 조건

### 에이전트 확인
- [x] 관련 코드 수정 완료
- [x] 로컬 정적 점검 또는 프로젝트 구조 기준 확인 완료
- [x] Unreal C++/Blueprint/에셋 규칙 위반 없음
- [x] 현재 task 문서가 실제 구현 기준으로 갱신됨

### 결과 확인
- Unreal Editor에서 `ComfyUIWorkflowRequestActor` 기반 Blueprint를 선택한다.
- PIE를 실행하지 않은 상태에서 Details 패널의 `Check Server Connection` 버튼을 눌러 `/system_stats` 응답이 오는지 확인한다.
- PIE를 실행하지 않은 상태에서 Details 패널의 `Send Workflow Prompt` 버튼을 누른다.
- ComfyUI 서버가 실행 중이고 workflow JSON이 올바를 때 `ComfyUI|Last Response`의 성공 여부, HTTP 상태 코드, 응답 메시지가 갱신되는지 확인한다.
- `ComfyUI|Last Override`에서 override가 적용된 긍정 프롬프트 노드 ID, 부정 프롬프트 노드 ID, 이미지 크기 노드 ID를 확인한다.
- Output Log에서 ComfyUI `/prompt` 응답 로그를 확인한다.
- ComfyUI 서버를 끄거나 잘못된 파일 경로를 지정했을 때 실패 사유가 `Last Response Message`와 Output Log에 표시되는지 확인한다.
- 지정한 프롬프트와 이미지 크기 값으로 저장된 workflow가 실행되는지 ComfyUI 쪽 실행 상태를 확인한다.
