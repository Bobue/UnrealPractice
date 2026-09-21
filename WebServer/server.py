"""Small dependency-free server directory used by the Unreal assignment."""

from __future__ import annotations

import argparse
import hmac
import json
import os
import threading
from datetime import datetime, timezone
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path
from typing import Any
from urllib.parse import urlsplit


MAX_REQUEST_BYTES = 16 * 1024


def is_valid_server_address(address: object) -> bool:
    if not isinstance(address, str) or not address or any(ch.isspace() for ch in address):
        return False
    try:
        parsed = urlsplit(f"//{address}")
        port = parsed.port
    except ValueError:
        return False
    return (
        bool(parsed.hostname)
        and port is not None
        and 1 <= port <= 65535
        and parsed.netloc == address
        and parsed.username is None
        and parsed.password is None
        and not parsed.path
        and not parsed.query
        and not parsed.fragment
    )


class ServerDirectory:
    """Owns the demo credentials and the most recently registered server."""

    def __init__(
        self,
        user_id: str,
        password: str,
        registration_token: str,
        registry_path: Path,
    ) -> None:
        self.user_id = user_id
        self.password = password
        self.registration_token = registration_token
        self.registry_path = registry_path
        self._lock = threading.Lock()
        self._server_address: str | None = None
        self._load()

    def credentials_match(self, user_id: object, password: object) -> bool:
        return (
            isinstance(user_id, str)
            and isinstance(password, str)
            and hmac.compare_digest(user_id, self.user_id)
            and hmac.compare_digest(password, self.password)
        )

    def token_matches(self, token: str) -> bool:
        return hmac.compare_digest(token, self.registration_token)

    def register(self, server_address: str) -> None:
        if not is_valid_server_address(server_address):
            raise ValueError("Invalid serverAddress")
        record = {
            "serverAddress": server_address,
            "registeredAt": datetime.now(timezone.utc).isoformat(),
        }
        with self._lock:
            self.registry_path.parent.mkdir(parents=True, exist_ok=True)
            temporary_path = self.registry_path.with_suffix(self.registry_path.suffix + ".tmp")
            temporary_path.write_text(
                json.dumps(record, ensure_ascii=False, indent=2), encoding="utf-8"
            )
            os.replace(temporary_path, self.registry_path)
            self._server_address = server_address

    def current_server(self) -> str | None:
        with self._lock:
            return self._server_address

    def _load(self) -> None:
        try:
            record = json.loads(self.registry_path.read_text(encoding="utf-8"))
            address = record.get("serverAddress")
            if is_valid_server_address(address):
                self._server_address = address
        except (FileNotFoundError, OSError, json.JSONDecodeError, AttributeError):
            self._server_address = None


class DirectoryHttpServer(ThreadingHTTPServer):
    daemon_threads = True

    def __init__(self, address: tuple[str, int], directory: ServerDirectory) -> None:
        super().__init__(address, DirectoryRequestHandler)
        self.directory = directory


class DirectoryRequestHandler(BaseHTTPRequestHandler):
    server: DirectoryHttpServer

    def do_GET(self) -> None:  # noqa: N802 - BaseHTTPRequestHandler API
        if self.path == "/health":
            self._send_json(200, {"status": "ok"})
            return
        self._send_json(404, {"success": False, "message": "Not found"})

    def do_POST(self) -> None:  # noqa: N802 - BaseHTTPRequestHandler API
        try:
            payload = self._read_json()
        except ValueError as error:
            self._send_json(400, {"success": False, "message": str(error)})
            return

        if self.path == "/api/server/register":
            self._register(payload)
        elif self.path == "/api/login":
            self._login(payload)
        else:
            self._send_json(404, {"success": False, "message": "Not found"})

    def _register(self, payload: dict[str, Any]) -> None:
        authorization = self.headers.get("Authorization", "")
        token = authorization.removeprefix("Bearer ") if authorization.startswith("Bearer ") else ""
        if not self.server.directory.token_matches(token):
            self._send_json(401, {"success": False, "message": "Unauthorized"})
            return

        address = payload.get("serverAddress")
        if not is_valid_server_address(address):
            self._send_json(400, {"success": False, "message": "Invalid serverAddress"})
            return

        self.server.directory.register(address)
        self._send_json(200, {"success": True, "serverAddress": address})

    def _login(self, payload: dict[str, Any]) -> None:
        if not self.server.directory.credentials_match(
            payload.get("userId"), payload.get("password")
        ):
            self._send_json(401, {"success": False, "message": "Invalid credentials"})
            return

        address = self.server.directory.current_server()
        if address is None:
            self._send_json(404, {"success": False, "message": "No server is registered"})
            return
        self._send_json(200, {"success": True, "serverAddress": address})

    def _read_json(self) -> dict[str, Any]:
        try:
            content_length = int(self.headers.get("Content-Length", "0"))
        except ValueError as error:
            raise ValueError("Invalid Content-Length") from error
        if content_length <= 0 or content_length > MAX_REQUEST_BYTES:
            raise ValueError("Invalid request body size")
        try:
            payload = json.loads(self.rfile.read(content_length).decode("utf-8"))
        except (UnicodeDecodeError, json.JSONDecodeError) as error:
            raise ValueError("Malformed JSON") from error
        if not isinstance(payload, dict):
            raise ValueError("JSON body must be an object")
        return payload

    def _send_json(self, status: int, payload: dict[str, Any]) -> None:
        body = json.dumps(payload, ensure_ascii=False).encode("utf-8")
        self.send_response(status)
        self.send_header("Content-Type", "application/json; charset=utf-8")
        self.send_header("Content-Length", str(len(body)))
        self.end_headers()
        self.wfile.write(body)

    def log_message(self, message_format: str, *args: object) -> None:
        print(f"[{self.log_date_time_string()}] {self.address_string()} {message_format % args}")


def create_server(host: str, port: int, directory: ServerDirectory) -> DirectoryHttpServer:
    return DirectoryHttpServer((host, port), directory)


def main() -> None:
    parser = argparse.ArgumentParser(description="Unreal assignment server directory")
    parser.add_argument("--host", default=os.getenv("MATCHMAKING_HOST", "0.0.0.0"))
    parser.add_argument(
        "--port", type=int, default=int(os.getenv("MATCHMAKING_PORT", "8080"))
    )
    parser.add_argument(
        "--data-file",
        type=Path,
        default=Path(__file__).parent / "data" / "server_registry.json",
    )
    args = parser.parse_args()

    directory = ServerDirectory(
        user_id=os.getenv("MATCHMAKING_USER", "student"),
        password=os.getenv("MATCHMAKING_PASSWORD", "1234"),
        registration_token=os.getenv("UNREAL_SERVER_TOKEN", "assignment-secret"),
        registry_path=args.data_file,
    )
    server = create_server(args.host, args.port, directory)
    print(f"Server directory listening on http://{args.host}:{server.server_port}")
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        print("Stopping server directory")
    finally:
        server.server_close()


if __name__ == "__main__":
    main()
