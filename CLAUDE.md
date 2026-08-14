# CLAUDE.md — 에테르나 (Aeterna)

@AGENTS.md

> 위 AGENTS.md가 이 저장소의 단일 규칙 원본이다. 아래는 Claude Code 사용 시 추가 지침.

## 프로젝트 한 줄 요약

UE **5.6** / 1인칭 규칙 기반 호러 「에테르나」. 플레이어는 로봇 M-05, 조작은 5입력(WASD·Shift·Tab·E·F)뿐,
수첩 규칙 위반만이 게임 오버이며 밤 완료 시 페이드아웃 → 「Day N」 카드로 넘어간다.
기획 원본: `Docs/GDD_v1.3_에테르나.md`

## Claude Code 작업 순서

1. 작업 전 `Docs/GDD_v1.3_에테르나.md`에서 관련 섹션을 읽는다 (특히 §2 조작, §3 배터리, §5 시스템, §6 시나리오).
2. AGENTS.md §3 **Aeterna 기획 기준(불변 조건 6개)**을 위반하는 코드를 절대 생성하지 않는다. 사용자가 요청하더라도 충돌을 알리고 확인을 받는다.
3. C++/BP 경계를 지킨다: 판정·상태·저장은 `Source/Aeterna/` C++, 연출은 BP. Claude는 C++와 CSV/텍스트만 직접 수정하고, `.uasset`/`.umap`은 에디터 내 수정 절차를 안내만 한다.
4. 커밋 시 접두어 필수: `[C++]` `[BP]` `[Map]` `[Data]` `[Doc]`.

> 금지 목록(입력 추가·새 실패 경로·경고 UI·경비실 복귀 로직·하드코딩·무단 구현 등)은 **AGENTS.md §3과 §5에만** 둔다. 여기에 다시 적지 않는다 — 두 곳에 적으면 한쪽만 고쳐져 충돌한다.

## 저장소 판별 (먼저 확인)

이 프로젝트의 문서는 Unreal 프로젝트 저장소와 별도 폴더에 있을 수 있다. **작업 시작 시 현재 폴더가 어느 쪽인지 먼저 확인한다.**

| 현재 폴더 | 판별 | Claude가 하는 일 |
|---|---|---|
| **기획·문서 폴더** | `.uproject` 없음 / Git 저장소 아님 | 문서 편집만. `git lfs` 절차·커밋 접두어 규칙은 적용하지 않는다 |
| **Unreal 프로젝트 저장소** | `Aeterna.uproject` + `.git` 존재 | 아래 명령과 담당 확인 절차를 전부 적용 |

Git 저장소가 아닌 곳에서 `git lfs locks`를 실행하려 하지 않는다. 담당 확인이 불가능하면 **확인 불가라고 밝히고** 진행 여부를 사용자에게 묻는다.

## 자주 쓰는 명령 (Unreal 프로젝트 저장소에서만)

```bash
# 빌드 (에디터 타깃)
# Windows 기준 — 경로는 각자 환경에 맞게
"C:/Program Files/Epic Games/UE_5.6/Engine/Build/BatchFiles/Build.bat" AeternaEditor Win64 Development -project="<repo>/Aeterna.uproject"

# 바이너리 잠금/해제
git lfs lock "Content/Aeterna/Maps/Sub_GrandHall.umap"
git lfs locks
git lfs unlock "Content/Aeterna/Maps/Sub_GrandHall.umap"
```

## 작업 조율

담당 구분은 **고정 담당제**이며 AGENTS.md §2 (원본: GDD §9)를 따른다. Claude는 작업 전 다음을 확인한다:

1. 수정 대상이 **어느 담당 영역(A 시스템 / B 플레이어 / C 레벨 / D 시나리오)**에 속하는지 판별한다
2. Unreal 저장소라면 `git lfs locks`와 최근 커밋 로그로 잠금·진행 상황을 확인한다
3. 사용자 담당 밖의 영역이거나 타인이 작업 중이면 — 수정하지 말고 "OOO 담당 영역입니다. 합의 후 진행하세요"라고 알린 뒤, 그 담당자에게 넘길 **변경 제안 목록만** 만들어 준다
4. 담당 경계에 걸치는 기능은 양쪽 인터페이스를 먼저 제시하고 합의를 요청한다

## 문체 가이드 (게임 내 텍스트 생성 시)

- **수첩**: 낡은 손글씨 경어체 — "~하시오 / ~마시오". 짧고 단정적, 이유는 설명하지 않음.
- **시스템 로그**: 기계식 영문 대문자 헤더 + 한국어 단문 — 예: `NIGHT EXHIBIT CHECK: ACTIVE`, `등록되지 않은 변경 사항`.
- 공포는 서술하지 않고 암시한다. 실체를 묘사하는 문장을 쓰지 않는다.
