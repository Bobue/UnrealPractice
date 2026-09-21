import json
import tempfile
import threading
import unittest
import urllib.error
import urllib.request
from pathlib import Path

from WebServer.server import ServerDirectory, create_server


class ServerApiTests(unittest.TestCase):
    def setUp(self):
        self.temp_dir = tempfile.TemporaryDirectory()
        registry_path = Path(self.temp_dir.name) / "registry.json"
        self.directory = ServerDirectory(
            user_id="student",
            password="1234",
            registration_token="assignment-secret",
            registry_path=registry_path,
        )
        self.server = create_server("127.0.0.1", 0, self.directory)
        self.thread = threading.Thread(target=self.server.serve_forever, daemon=True)
        self.thread.start()
        self.base_url = f"http://127.0.0.1:{self.server.server_port}"

    def tearDown(self):
        self.server.shutdown()
        self.server.server_close()
        self.thread.join(timeout=2)
        self.temp_dir.cleanup()

    def request(self, method, path, payload=None, token=None):
        data = None if payload is None else json.dumps(payload).encode("utf-8")
        request = urllib.request.Request(
            self.base_url + path,
            data=data,
            method=method,
            headers={"Content-Type": "application/json"},
        )
        if token is not None:
            request.add_header("Authorization", f"Bearer {token}")
        try:
            with urllib.request.urlopen(request, timeout=2) as response:
                return response.status, json.loads(response.read().decode("utf-8"))
        except urllib.error.HTTPError as error:
            return error.code, json.loads(error.read().decode("utf-8"))

    def raw_request(self, path, body):
        request = urllib.request.Request(
            self.base_url + path,
            data=body,
            method="POST",
            headers={"Content-Type": "application/json"},
        )
        try:
            with urllib.request.urlopen(request, timeout=2) as response:
                return response.status, json.loads(response.read().decode("utf-8"))
        except urllib.error.HTTPError as error:
            return error.code, json.loads(error.read().decode("utf-8"))

    def test_health_reports_ok(self):
        status, body = self.request("GET", "/health")
        self.assertEqual(200, status)
        self.assertEqual({"status": "ok"}, body)

    def test_registration_then_login_returns_server_address(self):
        status, body = self.request(
            "POST",
            "/api/server/register",
            {"serverAddress": "192.168.0.10:7777"},
            token="assignment-secret",
        )
        self.assertEqual(200, status)
        self.assertEqual("192.168.0.10:7777", body["serverAddress"])

        status, body = self.request(
            "POST", "/api/login", {"userId": "student", "password": "1234"}
        )
        self.assertEqual(200, status)
        self.assertEqual(
            {"success": True, "serverAddress": "192.168.0.10:7777"}, body
        )

    def test_registration_rejects_wrong_token(self):
        status, body = self.request(
            "POST",
            "/api/server/register",
            {"serverAddress": "127.0.0.1:7777"},
            token="wrong-token",
        )
        self.assertEqual(401, status)
        self.assertEqual(False, body["success"])

    def test_login_rejects_invalid_credentials(self):
        status, body = self.request(
            "POST", "/api/login", {"userId": "student", "password": "wrong"}
        )
        self.assertEqual(401, status)
        self.assertEqual("Invalid credentials", body["message"])

    def test_login_reports_when_no_server_is_registered(self):
        status, body = self.request(
            "POST", "/api/login", {"userId": "student", "password": "1234"}
        )
        self.assertEqual(404, status)
        self.assertEqual("No server is registered", body["message"])

    def test_registration_rejects_invalid_address(self):
        for address in (
            "not-an-address",
            "user@game.example.com:7777",
            "game.example.com:7777?option=bad",
            "game.example.com:0",
        ):
            with self.subTest(address=address):
                status, body = self.request(
                    "POST",
                    "/api/server/register",
                    {"serverAddress": address},
                    token="assignment-secret",
                )
                self.assertEqual(400, status)
                self.assertEqual("Invalid serverAddress", body["message"])

    def test_malformed_json_is_rejected_without_mutating_registry(self):
        status, body = self.raw_request("/api/server/register", b"{broken")
        self.assertEqual(400, status)
        self.assertEqual("Malformed JSON", body["message"])
        self.assertIsNone(self.directory.current_server())

    def test_registry_is_restored_from_disk(self):
        self.directory.register("game.example.com:7777")
        restored = ServerDirectory(
            user_id="student",
            password="1234",
            registration_token="assignment-secret",
            registry_path=Path(self.temp_dir.name) / "registry.json",
        )
        self.assertEqual("game.example.com:7777", restored.current_server())


if __name__ == "__main__":
    unittest.main()
