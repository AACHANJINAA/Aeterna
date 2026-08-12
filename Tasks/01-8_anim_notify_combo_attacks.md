# Task 01-8 - Anim Notify 기반 3단 공격 애니메이션

## 설명
좌클릭 3단 콤보 사격의 1타, 2타, 3타 단계에 맞춰 공격 애니메이션이 재생되도록 연결한다. 다음 콤보 입력 가능 여부는 단순 시간값이 아니라 공격 애니메이션의 Anim Notify 구간을 기준으로 판단되어야 하며, 기존 피해량, 명중 판정, 피해 적용 흐름은 유지한다.

## 구현 항목
- [x] 좌클릭 사격 콤보 1타, 2타, 3타에 맞춰 각각의 공격 몽타주 또는 몽타주 섹션이 재생된다.
- [x] 각 콤보 애니메이션 후반부의 Anim Notify 구간에서만 다음 콤보 입력을 받을 수 있다.
- [x] 입력 허용 Notify 이전에 들어온 좌클릭은 다음 콤보 단계로 전환하지 않는다.
- [x] 입력 허용 구간 안에 좌클릭이 들어오면 즉시 다음 콤보 몽타주 또는 섹션으로 전환된다.
- [x] 입력 허용 구간을 지나도록 추가 좌클릭이 없으면 현재 콤보가 종료된다.
- [x] 콤보가 종료된 뒤 다음 좌클릭은 기존 리셋 규칙에 따라 1타부터 시작한다.
- [x] 3타 이후에는 다음 콤보 단계로 전환하지 않고 콤보가 종료된다.
- [x] 다음 공격으로 전환될 때 자연스럽게 보이도록 몽타주 Blend In, Blend Out 또는 섹션 전환 값 조정이 가능하다.
- [x] 기존 피해량, 명중 판정, 피해 적용 로직은 변경하지 않는다.

## 범위 밖
- 새 공격 애니메이션 제작
- 최종 전투 연출 품질 작업
- 신규 무기 시스템
- 신규 적 체력 또는 피격 반응 시스템
- HUD 콤보 표시 구현
- 탄창, 장전, 탄약 소모
- 사격 이펙트, 사운드, 카메라 연출의 최종 품질 작업

## 사전 전제
- 01-6 단계의 Wraith 메시와 기본 이동 로코모션 구성이 유지되어 있다.
- 01-7 단계의 좌클릭 3단 콤보 단계, 피해량, 리셋 규칙이 유지되어 있다.

## 수동 작업
- Unreal Editor에서 `BP_PlayerCubeCharacter`를 연다.
- Details 패널의 `Combat|Combo|Animation`에서 `FireComboStep1Montage`, `FireComboStep2Montage`, `FireComboStep3Montage`에 1타, 2타, 3타 공격 몽타주를 연결한다.
- 하나의 몽타주를 섹션으로 나눠 사용할 경우 각 단계에 같은 몽타주를 연결하고 `FireComboStep1SectionName`, `FireComboStep2SectionName`, `FireComboStep3SectionName`에 실제 섹션 이름을 입력한다.
- 필요하면 `FireComboMontagePlayRate`와 `FireComboMontageStopBlendOutTime`을 조정한다.
- 각 공격 몽타주의 다음 입력 허용 시작 지점에 Anim Notify를 추가하고 `OpenFireComboInputWindow`를 호출하도록 연결한다.
- 각 공격 몽타주의 다음 입력 허용 종료 지점에 Anim Notify를 추가하고 `CloseFireComboInputWindow`를 호출하도록 연결한다.
- 각 공격 몽타주의 공격 종료 지점에 Anim Notify를 추가하고 `EndFireComboAttack`을 호출하도록 연결한다.
- 3타 몽타주에는 다음 입력 허용 Notify를 두지 않거나, 두더라도 `OpenFireComboInputWindow`가 다음 단계 전환을 열지 않는지 확인한다.

## 완료 조건

### 에이전트 확인
- [x] 관련 코드 수정 완료
- [x] 로컬 정적 점검 또는 프로젝트 구조 기준 확인 완료
- [x] Unreal C++/Blueprint/에셋 규칙 위반 없음
- [x] 현재 task 문서가 실제 구현 기준으로 갱신됨

### 결과 확인
- [ ] Unreal Editor에서 `Content/Maps/L_BattleMap.umap`을 연다.
- [ ] PIE를 실행한다.
- [ ] 좌클릭을 한 번 눌렀을 때 1타 공격 몽타주 또는 섹션이 재생되고 Output Log에 `ComboStep=1`과 기존 사격 명중 판정 로그가 함께 나오는지 확인한다.
- [ ] 1타 입력 허용 Notify 이전에 좌클릭을 다시 눌렀을 때 `Fire input ignored by Anim Notify combo window` 로그가 나오고 2타로 전환되지 않는지 확인한다.
- [ ] 1타 입력 허용 구간 안에서 좌클릭을 다시 눌렀을 때 2타 공격 몽타주 또는 섹션으로 즉시 전환되고 Output Log에 `ComboStep=2`가 나오는지 확인한다.
- [ ] 2타 입력 허용 구간 안에서 좌클릭을 다시 눌렀을 때 3타 공격 몽타주 또는 섹션으로 즉시 전환되고 Output Log에 `ComboStep=3`이 나오는지 확인한다.
- [ ] 3타 도중 좌클릭을 다시 눌러도 다음 콤보 단계로 전환되지 않는지 확인한다.
- [ ] 입력 허용 구간을 지나도록 추가 좌클릭을 하지 않았을 때 `EndFireComboAttack` 이후 다음 좌클릭이 `ComboStep=1`부터 시작하는지 확인한다.
- [ ] 우클릭 조준 중에도 좌클릭 콤보 애니메이션, 기존 사격 방향, 명중 판정 로그가 유지되는지 확인한다.
