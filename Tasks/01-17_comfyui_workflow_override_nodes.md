# Task 01-17 - ComfyUI workflow 노드 자동 변경

## 설명
준비된 ComfyUI workflow 파일에서 긍정 프롬프트와 이미지 크기 값을 Unreal 쪽 입력값으로 바꿔 요청할 수 있는지 검증한다. workflow 안에는 긍정 프롬프트 값을 가지는 노드와 이미지 크기 값을 가지는 노드가 각각 하나씩 존재하는 것으로 전제한다. 에이전트는 workflow 구조에서 해당 노드 ID를 찾아, 사용자가 Unreal Editor에서 입력한 긍정 프롬프트와 이미지 크기 값이 실제 요청에 반영되도록 해야 한다.

## 구현 항목
- [x] 준비된 ComfyUI workflow JSON에서 긍정 프롬프트 값을 가진 노드 ID를 찾을 수 있다.
- [x] 준비된 ComfyUI workflow JSON에서 이미지 너비와 높이 값을 가진 노드 ID를 찾을 수 있다.
- [x] Unreal Editor에서 입력한 긍정 프롬프트 값이 찾은 긍정 프롬프트 노드에 반영된다.
- [x] Unreal Editor에서 입력한 이미지 너비와 높이 값이 찾은 이미지 크기 노드에 반영된다.
- [x] 노드 ID를 사용자가 직접 입력하지 않아도, 찾은 노드 ID와 변경 적용 결과를 Unreal Editor 또는 Output Log에서 확인할 수 있다.
- [x] 찾은 노드가 없거나 연결 관계로도 자동 판단할 수 없을 때 실패 사유를 확인할 수 있다.
- [x] 변경된 값으로 ComfyUI `/prompt` 요청이 전송된다.

## 범위 밖
- 부정 프롬프트 자동 변경
- 여러 긍정 프롬프트 노드 또는 여러 이미지 크기 노드가 있는 복잡한 workflow 자동 선택
- ComfyUI workflow 편집기 제작
- 이미지 생성 결과 다운로드, 저장, Unreal 에셋 임포트
- ComfyUI 설치 또는 실행 자동화

## 사전 전제
- 01-16 단계의 Unreal Editor에서 로컬 ComfyUI workflow 요청 검증 기능이 유지되어 있다.
- ComfyUI 서버는 로컬 PC에서 별도로 실행되어 있다.
- 요청에 사용할 ComfyUI workflow JSON 파일은 미리 준비되어 있다.

## 수동 작업
- Unreal Editor에서 C++ 변경 사항을 컴파일한다.
- `ComfyUIWorkflowRequestActor` 기반 Blueprint를 열거나 레벨에 배치한 뒤 선택한다.
- Details 패널의 `ComfyUI|Request` 값에서 `Workflow Json File`에 준비된 ComfyUI workflow JSON 파일을 지정한다.
- Details 패널의 `ComfyUI|Override` 값에서 `Positive Prompt Text`, `Image Width`, `Image Height`를 입력한다.
- 자동 판별을 확인할 때는 `Positive Prompt Node Id`와 `Image Size Node Id`를 `0`으로 둔다.

## 완료 조건

### 에이전트 확인
- [x] 관련 코드 수정 완료
- [x] 로컬 정적 점검 또는 프로젝트 구조 기준 확인 완료
- [x] Unreal C++/Blueprint/에셋 규칙 위반 없음
- [x] 현재 task 문서가 실제 구현 기준으로 갱신됨

### 결과 확인
- PIE를 실행하지 않은 상태에서 Details 패널의 `Check Server Connection` 버튼을 눌러 ComfyUI 서버 응답을 확인한다.
- PIE를 실행하지 않은 상태에서 Details 패널의 `Send Workflow Prompt` 버튼을 누른다.
- Output Log에서 `Override 적용 결과` 로그에 긍정 프롬프트 노드 ID와 이미지 크기 노드 ID가 `0`이 아닌 값으로 표시되는지 확인한다.
- `ComfyUI|Last Override`에서 적용된 긍정 프롬프트 노드 ID와 이미지 크기 노드 ID를 확인한다.
- ComfyUI에서 생성된 이미지가 Unreal Editor에서 입력한 긍정 프롬프트와 이미지 크기 값으로 생성됐는지 확인한다.
- `CLIPTextEncode`가 둘 이상 있어도 `KSampler`의 `positive` 입력에 연결된 노드가 긍정 프롬프트 노드로 선택되는지 확인한다.
- workflow에 이미지 크기 후보가 없거나 둘 이상이면 자동 판단 실패 메시지가 표시되는지 확인한다.
