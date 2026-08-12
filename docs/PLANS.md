# Plans

## 목적

이 문서는 현재 진행 중인 단계 상태와 최근 작업 흐름을 빠르게 확인하기 위한 기록 문서입니다.
<!-- 새 프로젝트 시작 시 작업리스트와 최근 작업 로그의 기록을 초기화한다. -->

## 작업리스트
| 단계 | Task 문서 | 해야 할 항목 | 상태 |
| --- | --- | --- |----|
| 01-1 | [Tasks/01-1_basic_play_map.md](../Tasks/01-1_basic_play_map.md) | 동작 검증용 기본 플레이 맵 구성 | 완료 |
| 01-2 | [Tasks/01-2_player_cube_movement.md](../Tasks/01-2_player_cube_movement.md) | WASD 입력으로 플레이어 큐브 이동 검증 | 완료 |
| 01-3 | [Tasks/01-3_player_character_base.md](../Tasks/01-3_player_character_base.md) | 플레이어 조작 대상을 Character 기반 구조로 전환 | 완료 |
| 01-4 | [Tasks/01-4_third_person_shoulder_camera.md](../Tasks/01-4_third_person_shoulder_camera.md) | 마우스 조작 3인칭 숄더뷰 카메라와 카메라 기준 이동 구성 | 완료 |
| 01-5 | [Tasks/01-5_basic_aim_and_fire.md](../Tasks/01-5_basic_aim_and_fire.md) | 마우스 우클릭 조준과 좌클릭 기본 사격 명중 판정 검증 | 완료 |
| 01-6 | [Tasks/01-6_wraith_locomotion_mesh.md](../Tasks/01-6_wraith_locomotion_mesh.md) | 플레이어 큐브를 Paragon Wraith 메시와 기본 이동 로코모션으로 교체 | 완료 |
| 01-7 | [Tasks/01-7_three_step_fire_combo.md](../Tasks/01-7_three_step_fire_combo.md) | 좌클릭 연속 사격을 1타·2타·3타 콤보로 관리 | 완료 |
| 01-8 | [Tasks/01-8_anim_notify_combo_attacks.md](../Tasks/01-8_anim_notify_combo_attacks.md) | 3단 콤보 사격 단계별 공격 애니메이션과 Anim Notify 입력 허용 구간 연결 | 완료 |
| 01-9 | [Tasks/01-9_vertical_aim_offset.md](../Tasks/01-9_vertical_aim_offset.md) | 마우스 상하 움직임에 따른 상체 세로 조준 Aim Offset 검증 | 완료 |
| 01-10 | [Tasks/01-10_spacebar_jump.md](../Tasks/01-10_spacebar_jump.md) | 스페이스바 입력으로 플레이어 점프와 공중 단일 모션 검증 | 완료 |
| 01-11 | [Tasks/01-11_boss_cube_chase_movement.md](../Tasks/01-11_boss_cube_chase_movement.md) | 보스 큐브의 길찾기 기반 플레이어 추적 이동 검증 | 완료 |
| 01-12 | [Tasks/01-12_boss_melee_attack_state.md](../Tasks/01-12_boss_melee_attack_state.md) | 보스의 근접 일반 공격 상태 전환 검증 | 완료 |
| 01-13 | [Tasks/01-13_boss_jump_slam_attack.md](../Tasks/01-13_boss_jump_slam_attack.md) | 보스의 거리 기반 점프 내려찍기 공격 검증 | 완료 |
| 01-14 | [Tasks/01-14_boss_fsm_animation_state.md](../Tasks/01-14_boss_fsm_animation_state.md) | 보스 FSM 상태에 맞는 애니메이션 상태 전환 검증 | 완료 |
| 01-15 | [Tasks/01-15_circular_boss_battle_arena.md](../Tasks/01-15_circular_boss_battle_arena.md) | 산과 절벽으로 둘러싸인 원형 보스 전투장 검증 | 완료 |
| 01-16 | [Tasks/01-16_local_comfyui_workflow_request.md](../Tasks/01-16_local_comfyui_workflow_request.md) | Unreal Editor에서 로컬 ComfyUI workflow 요청 검증 | 완료 |
| 01-17 | [Tasks/01-17_comfyui_workflow_override_nodes.md](../Tasks/01-17_comfyui_workflow_override_nodes.md) | ComfyUI workflow의 프롬프트와 이미지 크기 노드 자동 변경 검증 | 진행중 |

## 상태 범례

- `예정`: 아직 시작 전
- `진행중`: 구현 중이거나, 구현 후 사용자 수동 작업 또는 결과 확인을 기다리는 상태
- `완료`: 구현 및 현재 확인 모드 기준 확인 완료

## 최근 작업 로그
- 2026-08-12: 01-16 ComfyUI UI workflow 저장본을 API prompt JSON으로 착각해 보내는 경우를 감지해 로컬 실패 메시지를 표시하도록 보정.
- 2026-08-12: 01-16 저장된 ComfyUI UI workflow JSON을 `/prompt` API용 prompt 객체로 변환하고 프롬프트와 이미지 크기 override를 적용하도록 변경.
- 2026-08-12: 01-16 ComfyUI 검증 응답에 맞춰 `UNETLoader`, `CLIPLoader`, `InspyrenetRembg` widget 입력 매핑을 추가.
- 2026-08-12: 01-16 저장된 workflow의 원본 프롬프트가 실행되는 경우를 구분하기 위해 override가 실제 적용된 노드 ID와 메시지를 노출.
- 2026-08-12: 사용자 요청에 따라 01-17 ComfyUI workflow의 프롬프트와 이미지 크기 노드 자동 변경 검증 task 문서 작성.
- 2026-08-12: 01-17 ComfyUI workflow에서 `CLIPTextEncode`와 `EmptyLatentImage`가 정확히 하나일 때 자동으로 노드 ID를 확정해 override하도록 보정, 사용자 결과 확인 대기.
- 2026-08-12: 01-17 `CLIPTextEncode`가 여러 개일 때 `KSampler`의 `positive` 입력 링크를 따라 긍정 프롬프트 노드를 찾도록 보정.
