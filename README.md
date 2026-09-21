# PractiveUnreal AI Server Directory

언리얼 서버가 웹서버에 접속 주소를 등록하고, 클라이언트가 과제용 계정으로 로그인해 해당 주소로 자동 접속하는 예제입니다.

## 구성

- `WebServer/server.py`: 서버 주소 등록 및 로그인 API
- `UServerDirectorySubsystem`: 언리얼 HTTP 통신과 서버 이동 처리
- `UTitleWidgetBase`: 기존 시작/접속 버튼을 새 통신 기능에 연결
- `docs/ASSIGNMENT_REPORT.md`: 과제 제출용 작업 보고서

## 빠른 실행

Python 3.10 이상에서 웹서버를 먼저 실행합니다.

```powershell
python WebServer/server.py
```

기본 설정은 다음과 같습니다.

- 웹 API: `http://127.0.0.1:8080`
- ID: `student`
- 비밀번호: `1234`
- 서버 등록 토큰: `assignment-secret`
- 언리얼 접속 주소: `127.0.0.1:7777`

언리얼 에디터에서 서버 창과 클라이언트 창을 실행합니다. 서버 창에서 `StartServerButton`을 누르면 주소 등록 후 `Lobby?listen`으로 이동합니다. 클라이언트 창에서 ID와 비밀번호를 입력하고 `ConnectServerButton`을 누르면 웹 API가 반환한 주소로 접속합니다.

에디터 없이 창 두 개로 시연하려면 다음 명령을 두 번 실행합니다.

```powershell
UnrealEditor-Cmd.exe "PractiveUnreal.uproject" /Game/Map/Title -game -windowed -ResX=900 -ResY=500 -unattended
```

버튼 대신 콘솔 명령 `TitleStartServer`, `TitleLogin student 1234`로도 같은 흐름을 실행할 수 있습니다.

다른 PC에서 접속할 때는 호스트 게임을 다음 인자와 함께 실행합니다.

```text
-PublicServerAddress=192.168.0.10:7777
```

클라이언트와 호스트의 `Config/DefaultGame.ini`에서 `WebServerBaseUrl`을 웹서버 PC의 LAN 주소로 변경해야 합니다. Windows 방화벽에서도 TCP 8080과 UDP 7777을 허용해야 합니다.

## 주의사항

`Config/DefaultGame.ini`에서 `WebServerBaseUrl` 값은 반드시 따옴표로 감싸야 합니다. 언리얼 ini 파서가 따옴표 없는 `//`를 주석으로 처리해 주소가 `http:`로만 읽힙니다.

```ini
WebServerBaseUrl="http://127.0.0.1:8080"
```

## 테스트

```powershell
python -m unittest discover -s WebServer/tests -v
```

언리얼 테스트 이름은 `PractiveUnreal.ServerDirectory`입니다. Session Frontend의 Automation 탭 또는 다음 명령으로 실행할 수 있습니다.

```powershell
UnrealEditor-Cmd.exe PractiveUnreal.uproject -unattended -NullRHI -ExecCmds="Automation RunTests PractiveUnreal.ServerDirectory;Quit"
```

자세한 API와 설정은 [WebServer/README.md](WebServer/README.md), 작업 내용은 [docs/ASSIGNMENT_REPORT.md](docs/ASSIGNMENT_REPORT.md)를 참고하세요.
