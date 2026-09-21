# Assignment Web Server

외부 패키지 없이 Python 표준 라이브러리만 사용하는 단일 서버 디렉터리입니다.

## 실행

```powershell
python WebServer/server.py --host 0.0.0.0 --port 8080
```

환경변수로 과제용 인증 값을 바꿀 수 있습니다.

```powershell
$env:MATCHMAKING_USER = "student"
$env:MATCHMAKING_PASSWORD = "1234"
$env:UNREAL_SERVER_TOKEN = "assignment-secret"
python WebServer/server.py
```

등록된 주소는 `WebServer/data/server_registry.json`에 저장되며 Git에는 포함되지 않습니다.

## API

### 상태 확인

```http
GET /health
```

응답: `200 {"status":"ok"}`

### 언리얼 서버 등록

```http
POST /api/server/register
Authorization: Bearer assignment-secret
Content-Type: application/json

{"serverAddress":"127.0.0.1:7777"}
```

### 로그인 및 서버 조회

```http
POST /api/login
Content-Type: application/json

{"userId":"student","password":"1234"}
```

성공 응답:

```json
{"success": true, "serverAddress": "127.0.0.1:7777"}
```

오류 응답은 `success: false`와 `message`를 포함합니다. 잘못된 요청은 400, 인증 실패는 401, 등록 서버 없음은 404를 사용합니다.

## 테스트

```powershell
python -m unittest discover -s WebServer/tests -v
```
