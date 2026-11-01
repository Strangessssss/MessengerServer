"""Shared fixtures. pytest loads this file by itself and gives its fixtures to every test in this folder."""
import os
import socket
import subprocess
import time
from pathlib import Path

import pytest
import requests

REPO_ROOT = Path(__file__).resolve().parents[2]
JWT_SECRET = "module-tests-secret-with-at-least-32-characters"
DEFAULT_PASSWORD = "password123"
STARTUP_TIMEOUT_SECONDS = 10


def find_server_binary():
    """Returns MESSENGER_SERVER_BIN if set, or else the newest binary in build/ and cmake-build-*/."""
    configured = os.environ.get("MESSENGER_SERVER_BIN")
    if configured:
        return Path(configured)

    names = ("MessengerServer", "MessengerServer.exe")
    build_dirs = [REPO_ROOT / "build", *REPO_ROOT.glob("cmake-build-*")]
    binaries = [folder / name for folder in build_dirs for name in names if (folder / name).is_file()]
    if not binaries:
        pytest.fail("Server binary not found. Build it first, or set MESSENGER_SERVER_BIN.")
    return max(binaries, key=lambda path: path.stat().st_mtime)


def find_free_port():
    """Asks the OS for an unused port, so a server already running on 8080 can't get in the way."""
    with socket.socket() as probe:
        probe.bind(("127.0.0.1", 0))
        return probe.getsockname()[1]


def wait_until_listening(process, port, log_path):
    """A started process isn't ready yet, so keep trying to connect until it accepts."""
    deadline = time.monotonic() + STARTUP_TIMEOUT_SECONDS
    while time.monotonic() < deadline:
        if process.poll() is not None:
            pytest.fail(f"Server exited with code {process.returncode}:\n{log_path.read_text()}")
        try:
            socket.create_connection(("127.0.0.1", port), timeout=0.1).close()
            return
        except OSError:
            time.sleep(0.01)
    pytest.fail(f"Server didn't start listening on port {port}:\n{log_path.read_text()}")


class Api:
    """A thin wrapper over requests, so tests only say what's different about each call."""

    def __init__(self, base_url):
        self.base_url = base_url
        self.session = requests.Session()
        self.session.trust_env = False  # Never send local test traffic through a system proxy.

    def request(self, method, path, token=None, **kwargs):
        headers = kwargs.pop("headers", {})
        if token is not None:
            headers["Authorization"] = f"Bearer {token}"
        return self.session.request(method, self.base_url + path, headers=headers, timeout=5, **kwargs)

    def get(self, path, **kwargs):
        return self.request("GET", path, **kwargs)

    def post(self, path, **kwargs):
        return self.request("POST", path, **kwargs)

    def delete(self, path, **kwargs):
        return self.request("DELETE", path, **kwargs)

    def register(self, username, password=DEFAULT_PASSWORD):
        """Creates a user and returns its {"userId", "token"}. For tests where registering is only the setup."""
        response = self.post("/auth/register", json={"username": username, "password": password})
        assert response.status_code == 201, response.text
        return response.json()


class ServerProcess:
    """The real server binary, which a test can restart with the same JWT_SECRET."""

    def __init__(self, log_path):
        self.log_path = log_path
        self.process = None
        self.api = Api("")

    def start(self):
        port = find_free_port()
        env = {**os.environ, "JWT_SECRET": JWT_SECRET, "PORT": str(port)}

        # Logs go to a file because nobody would read a pipe, and a full pipe blocks the server.
        with self.log_path.open("a") as log:
            self.process = subprocess.Popen([find_server_binary()], env=env, stdout=log, stderr=subprocess.STDOUT)
        wait_until_listening(self.process, port, self.log_path)
        self.api.base_url = f"http://127.0.0.1:{port}"

    def stop(self):
        if self.process is not None:
            self.process.kill()
            self.process.wait()
            self.process = None

    def restart(self):
        self.stop()
        self.start()


@pytest.fixture
def server(tmp_path):
    """Starts a new server for one test, and stops it afterwards.

    The server keeps its data in memory, so a new process means an empty database. Every test
    can register "alice" without caring what the other tests did.
    """
    process = ServerProcess(tmp_path / "server.log")
    try:
        process.start()
        yield process  # The test runs here.
    finally:
        process.stop()


@pytest.fixture
def api(server):
    return server.api
