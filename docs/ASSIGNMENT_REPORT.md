# AI 기반 언리얼 서버 디렉터리 연동 작업 보고서

## 1. 과제 목표

기존 언리얼 프로젝트에 다음 흐름을 추가했습니다.

1. 언리얼 서버를 시작하면 웹서버에 접속 가능한 IP와 포트를 등록합니다.
2. 클라이언트가 과제용 계정으로 로그인하면 웹서버가 등록된 서버 주소를 반환합니다.
3. 클라이언트는 반환된 주소를 이용해 언리얼 서버에 자동 접속합니다.

## 2. 구현 구조

웹서버는 Python 표준 라이브러리의 `ThreadingHTTPServer`를 사용했습니다. 별도의 패키지 설치 없이 실행할 수 있고, 마지막으로 등록된 서버 주소를 JSON 파일에 저장합니다.

언리얼 쪽에는 `UServerDirectorySubsystem`을 추가했습니다. 게임 인스턴스 동안 유지되므로 맵 이동 전후에도 설정과 HTTP 기능을 한 곳에서 관리할 수 있습니다. 기존 `UTitleWidgetBase`는 입력값을 서브시스템에 전달하고 진행 결과를 화면 또는 로그에 표시합니다.

```text
언리얼 서버 ── POST /api/server/register ──> Python 웹서버
                                                   │
언리얼 클라이언트 ── POST /api/login ──────────────┘
        │
        └── ClientTravel("등록된 IP:7777") ──> 언리얼 서버
```

## 3. 주요 작업 내용

### 웹서버

- `GET /health` 상태 확인 API
- `POST /api/server/register` 서버 주소 등록 API
- Bearer 토큰을 이용한 서버 등록 보호
- `POST /api/login` 과제용 로그인 및 서버 주소 반환 API
- 주소 형식, JSON 본문, 요청 크기 검증
- 임시 파일과 원자적 교체를 이용한 JSON 저장
- 재시작 시 마지막 서버 주소 복구

### 언리얼

- `HTTP`, `Json`, `JsonUtilities`, `UMG` 모듈 의존성 추가
- 서버 등록 및 로그인 요청 구현
- `-PublicServerAddress=IP:PORT` 실행 인자 지원
- 등록 성공 후 `Lobby?listen` 맵 이동
- 전용 서버 실행 시 자동 등록
- 로그인 성공 후 `ClientTravel` 자동 호출
- HTTP 실패, 인증 실패, 서버 미등록, 잘못된 응답 처리
- 기존 타이틀 위젯의 서버 시작 및 접속 버튼 연동
- 선택형 `StatusText` 위젯과 기존 `ServerIP` 입력칸을 이용한 상태 표시

## 4. 설정값

`Config/DefaultGame.ini`에서 다음 값을 변경할 수 있습니다.

```ini
[/Script/PractiveUnreal.ServerDirectorySubsystem]
WebServerBaseUrl=http://127.0.0.1:8080
RegistrationToken=assignment-secret
DefaultServerAddress=127.0.0.1:7777
ListenMapName=Lobby
```

과제 기본 계정은 `student / 1234`입니다. 웹서버 환경변수 `MATCHMAKING_USER`, `MATCHMAKING_PASSWORD`, `UNREAL_SERVER_TOKEN`으로 변경할 수 있습니다. `RegistrationToken`과 `UNREAL_SERVER_TOKEN`은 동일해야 합니다.

## 5. 실행 및 시연 순서

1. `python WebServer/server.py`로 웹서버를 실행합니다.
2. 브라우저 또는 API 도구에서 `http://127.0.0.1:8080/health`가 `{"status":"ok"}`를 반환하는지 확인합니다.
3. 언리얼 에디터에서 두 개의 플레이어 창을 실행합니다.
4. 첫 번째 창에서 서버 시작 버튼을 누릅니다.
5. 웹서버 콘솔에서 `/api/server/register` 요청이 200으로 처리되는지 확인합니다.
6. 두 번째 창에서 ID `student`, 비밀번호 `1234`를 입력하고 접속 버튼을 누릅니다.
7. `/api/login` 요청 후 두 번째 창이 첫 번째 창의 Lobby 서버에 접속하는지 확인합니다.

LAN 시연에서는 서버 실행 인자로 `-PublicServerAddress=<호스트 LAN IP>:7777`을 전달하고 `WebServerBaseUrl`을 웹서버 PC 주소로 변경합니다.

## 6. 테스트 결과

- Python 실제 HTTP 통합 테스트: 상태 확인, 정상 등록/로그인, 잘못된 토큰, 잘못된 계정, 서버 미등록, 잘못된 주소, 잘못된 JSON, 파일 복구 검증
- Unreal 자동화 테스트: 정상 로그인 응답, 오류 응답, 서버 주소 누락 응답, 잘못된 형식의 서버 주소 응답 검증 (4개 통과)
- Unreal Editor Win64 Development C++ 빌드 검증

## 7. AI 활용 내용

AI를 활용해 기존 C++ 및 위젯 연결 구조를 분석하고, 웹 API 계약과 언리얼 서브시스템 구조를 설계했습니다. 테스트를 먼저 작성해 기능 부재로 실패하는 것을 확인한 뒤 구현했으며, 컴파일 오류와 실행 로그를 기반으로 코드를 보정했습니다. 마지막으로 실행 방법, API, 시연 순서와 제약사항을 문서화했습니다.

## 8. 과제 범위와 향후 개선

이번 구현은 수업 시연용 고정 계정과 단일 서버 등록을 대상으로 합니다. 실제 서비스에서는 HTTPS, 비밀번호 해시, 데이터베이스 계정, 로그인 토큰, 여러 서버 목록, 만료 시간과 heartbeat, NAT 및 포트 포워딩 처리가 추가로 필요합니다.
