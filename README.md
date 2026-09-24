# Lockpick Reveal Bar (락픽 탐지 바)

자물쇠 따기 화면 위쪽에 새까만 바를 띄우는 SKSE 플러그인입니다. 락픽을 대고 있는 부분만 서서히 밝아지고, 밝아진 칸의 색이 스윗스팟과의 거리를 알려줍니다.

- 잠금해제 스킬(강화 효과 포함 현재 값)이 높을수록 한 번에 밝혀지는 범위가 넓고 속도가 빠릅니다.
- 자물쇠를 돌리는(힘을 주는) 동안에는 더 빨리 밝혀집니다.
- 색으로 방향을 추측하지 못하도록 새 자물쇠마다(기본값: 락픽이 부러질 때도) 다음을 무작위로 바꿉니다.
  - 팔레트 7종
  - 색반전: 스윗스팟이 "차가운" 끝 색으로 보임
  - 계단화: 그라데이션을 5~10단계로 끊음
  - 칸마다 고정된 노이즈
- 흰 세로선은 현재 락픽 위치입니다.
- 원본 `lockpickingmenu.swf`를 교체하지 않고 런타임에 그리므로, 자물쇠 UI 스킨 모드와 함께 쓸 수 있습니다.

## 요구 사항

DLL 하나로 아래 세 버전을 모두 지원합니다. 게임 버전에 맞는 SKSE64와 Address Library for SKSE Plugins가 필요합니다.

| 게임 버전 | SKSE64 | Address Library | 상태 |
|---|---|---|---|
| SE 1.5.97 | 2.0.20 | SE용 (`version-1-5-97-0.bin`) | 지원 (인게임 미검증) |
| AE 1.6.1170 | 2.2.6 | AE용 (`versionlib-1-6-1170-0.bin`) | 인게임 테스트 완료 (v0.1.0, v0.2.0) |
| AE 1.7.104 (최신) | 2.3.1 | 1.7.104용 v13 (새 형식) | 지원 (인게임 미검증) |

## 설치

[Releases](https://github.com/servaltullius/LockpickRevealBar/releases)에서 `LockpickRevealBar-x.y.z.zip`을 받아 MO2의 "아카이브에서 모드 설치"로 설치하세요. 수동 설치라면 압축 안의 `SKSE` 폴더를 게임 `Data` 폴더에 복사하면 됩니다.

설정은 `SKSE\Plugins\LockpickRevealBar.ini`에 있고 항목마다 한글 주석이 달려 있습니다.

## 빌드

`build.bat`을 실행하면 configure → 빌드 → 테스트 → 패키징(`G:\skyrim-build\LockpickRevealBar\` 아래 폴더와 MO2용 zip)까지 진행됩니다. 필요한 환경은 VS 2022 v143과 `%USERPROFILE%\vcpkg`입니다. CommonLibSSE-NG([alandtse](https://github.com/alandtse/CommonLibSSE-NG) v9.0.2)는 `external/` 서브모듈로 들어 있으니, 처음 받았다면 `git submodule update --init`을 먼저 실행하세요. 1.7.x의 Address Library는 새 형식(v5)이라, CommonLibSSE-NG 3.7.0 이하로 빌드하면 1.7에서 로드에 실패합니다.

## 구현 메모

`LockpickingMenu` 필드는 SkyrimSE.exe 1.7.104를 디스어셈블해서 확인했습니다. 스윗스팟 판정 함수는 `pickAngle - center`를 `0.5 * width`, `partial`과 비교합니다.

| 필드 | legacy (SE/AE) | shifted (1.7.x) | 의미 |
|---|---|---|---|
| pickAngle | 0xDC | 0xDC | 락픽 각도, -90 ~ +90 |
| lockAngle | 0xE0 | 0xE0 | 자물쇠 회전량 (돌리는 중이면 0보다 큼) |
| unk0F8 | 0xF8 | 0x10C | 스윗스팟 **중심** (`[-90+w/2, 90-w/2]`에서 무작위) |
| sweetSpotAngle | 0xFC | 0x110 | 스윗스팟 **전체 폭** |
| partialPickAngle | 0x100 | 0x114 | 좌우 부분 회전 구간 폭 |
| numBrokenPicks | 0x104 | 0x118 | 이번 세션에서 부러진 락픽 수 |

1.7.x에서는 `pickBreakSeconds` 뒤에 0x14바이트가 추가되었습니다. 그래서 CommonLib의 `RUNTIME_DATA`를 그대로 쓰면 0xEC 이후 필드를 잘못 읽습니다. CommonLibSSE-NG v9도 1.7.99 이상에서 0x110부터 `float, float, uint32, bool...`이 있다고 기록하고 있는데, 이는 폭·부분 구간·부러진 개수가 밀려난 결과와 일치합니다. legacy 오프셋은 CommonLibSSE-NG 정의 기준입니다. 1.6.1170 exe는 Steam DRM으로 암호화되어 있어 디스어셈블로 대조하지는 못했고, 인게임 테스트로 확인했습니다(2026-09-24). 바가 가리킨 위치에서 전문가 자물쇠가 열렸고, 락픽이 부러질 때마다 스타일이 다시 굴려지는 것도 확인했습니다.

훅 대상은 `LockpickingMenu` vtable입니다.

- `ProcessMessage`(4): kShow에서 세션을 시작하고 kHide/kForceHide에서 정리합니다.
- `AdvanceMovie`(5): 매 프레임 갱신합니다.

바는 메뉴 무비의 `_root`에 `createEmptyMovieClip`으로 만든 칸별 클립에 AS2 드로잉 API로 그립니다. 색이 바뀐 칸만 다시 그립니다.

## 인게임 확인 체크리스트

1. `Documents\My Games\Skyrim Special Edition\SKSE\LockpickRevealBar.log`에 다음 두 줄이 있는지 확인합니다.
   - `Runtime 1.6.1170... -> sweet spot layout legacy(SE/AE)`
   - `LockpickingMenu hooks installed`
2. 자물쇠를 열면 `Lock ready: ... width=... partial=...` 줄이 찍히는지 확인합니다. 폭은 자물쇠 난이도와 스킬에 따라 다르며, 예를 들어 전문가 자물쇠에 스킬 10이면 0.56 정도였습니다.
3. 바가 검정으로 시작해 락픽 위치 주변만 밝아지는지 확인합니다.
4. `SweetSpotMarker=true`로 켜고, 표시된 칸에서 자물쇠가 끝까지 돌아가는지 확인합니다. 이게 레이아웃 검증입니다.
5. 바가 안 뜨면 `DebugLog=true`로 원시 값을 확인하고, 필요하면 `Layout=legacy|shifted`를 바꿔봅니다.

## 라이선스

이 저장소의 소스 코드는 [MIT](LICENSE)입니다. 배포되는 DLL은 GPL-3.0인 CommonLibSSE-NG를 정적 링크하므로, 바이너리 배포에는 GPL-3.0 조건이 적용됩니다.
