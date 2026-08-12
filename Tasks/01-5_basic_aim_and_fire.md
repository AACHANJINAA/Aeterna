# Task 01-5 - 기본 조준과 사격

## 설명
좌클릭으로 플레이어 정면 방향에 기본 사격을 발생시킨다. 마우스 우클릭을 누르면 확대 조준 상태에 들어가며, 탄이 나가는 방향과 명중 판정은 큐브 기반 검증 대상으로 확인할 수 있어야 한다.

## 구현 항목
- [x] 좌클릭으로 기본 사격이 발생한다.
- [x] 마우스 우클릭 또는 기본 TPS 조준 방식으로 확대 조준 상태에 들어갈 수 있다.
- [x] 조준 상태가 아니어도 좌클릭으로 기본 사격이 발생한다.
- [x] 사격 방향이 플레이어 정면 또는 현재 조준 방향 기준으로 확인 가능하다.
- [x] 큐브 기반 대상에 대한 명중 판정을 확인할 수 있다.
- [x] 실제 무기 모델이나 적 모델 없이 검증용 형상으로 조준, 사격 발생, 명중 판정을 확인할 수 있다.

## 범위 밖
- 피해량 적용
- 콤보 시스템
- 탄창, 장전, 탄약 소모
- 실제 무기 모델 연결
- 실제 적 모델 또는 적 AI 연결
- 사격 이펙트, 사운드, 카메라 연출의 최종 품질 작업
- 조준 전환 애니메이션 또는 보간 연출

## 사전 전제
- 01-4 단계의 3인칭 숄더뷰 카메라와 카메라 기준 이동이 동작한다.
- 플레이어 조작 대상이 `APlayerCubeCharacter` 기반 구조로 구성되어 있다.

## 수동 작업
- Unreal Editor에서 값 타입이 `Bool`인 조준용 Input Action을 만든다.
- Unreal Editor에서 값 타입이 `Bool`인 사격용 Input Action을 만든다.
- 기존 Input Mapping Context에 조준용 Input Action을 추가하고 마우스 우클릭을 매핑한다.
- 기존 Input Mapping Context에 사격용 Input Action을 추가하고 마우스 좌클릭을 매핑한다.
- `APlayerCubeCharacter` 기반 Blueprint에서 `AimAction`에 조준용 Input Action을 연결한다.
- `APlayerCubeCharacter` 기반 Blueprint에서 `FireAction`에 사격용 Input Action을 연결한다.
- `Content/Maps/L_BattleMap.umap`에 사격 명중 판정을 확인할 큐브 액터를 플레이어 정면에서 볼 수 있는 위치에 배치하고 저장한다.

## 완료 조건

### 에이전트 확인
- [x] 관련 코드 수정 완료
- [x] 로컬 정적 점검 또는 프로젝트 구조 기준 확인 완료
- [x] Unreal C++/Blueprint/에셋 규칙 위반 없음
- [x] 현재 task 문서가 실제 구현 기준으로 갱신됨

### 결과 확인
- [x] Unreal Editor에서 `Content/Maps/L_BattleMap.umap`을 연다.
- [x] PIE를 실행한다.
- [x] 마우스 우클릭을 누르면 화면이 더 확대되고 Output Log의 `Aim started` 로그가 나오는지 확인한다.
- [x] 마우스 우클릭을 놓으면 확대가 해제되고 Output Log의 `Aim stopped` 로그가 나오는지 확인한다.
- [x] 우클릭을 누르지 않은 상태에서 좌클릭하면 Output Log에 `Fire input received. IsAiming=false`가 나오고 카메라 정면으로 디버그 라인이 표시되는지 확인한다.
- [x] 우클릭을 누른 상태에서 좌클릭하면 Output Log에 `Fire input received. IsAiming=true`가 나오고 확대된 카메라 정면으로 디버그 라인이 표시되는지 확인한다.
- [x] 디버그 라인이 검증용 큐브를 맞출 때 녹색 선과 노란 점이 표시되고 Output Log에 `Fire hit` 로그가 나오는지 확인한다.
- [x] 검증용 큐브를 빗나가게 조준하고 좌클릭하면 빨간 선과 `Fire missed` 로그가 나오는지 확인한다.
- [x] 실제 무기 모델이나 적 모델 없이 큐브 형상으로 조준, 사격 발생, 명중 판정 검증이 되는지 확인한다.
