"""POST /auth/register, POST /auth/login, and how protected routes treat tokens."""
import pytest

from conftest import DEFAULT_PASSWORD


def test_register_returns_user_id_and_token(api):
    response = api.post("/auth/register", json={"username": "alice", "password": DEFAULT_PASSWORD})

    assert response.status_code == 201
    body = response.json()
    assert isinstance(body["userId"], int)
    assert body["token"]


def test_register_rejects_taken_username(api):
    api.register("alice")

    response = api.post("/auth/register", json={"username": "alice", "password": DEFAULT_PASSWORD})

    assert response.status_code == 400
    assert response.json() == {"error": "Username is taken or password is too short"}


@pytest.mark.parametrize(
    "body",
    [
        {"password": DEFAULT_PASSWORD},
        {"username": "alice"},
        {"username": 123, "password": DEFAULT_PASSWORD},
        {"username": "", "password": DEFAULT_PASSWORD},
        {"username": "alice", "password": "1234567"},
    ],
    ids=["no username", "no password", "username not a string", "empty username", "password too short"],
)
def test_register_rejects_invalid_body(api, body):
    response = api.post("/auth/register", json=body)

    assert response.status_code == 400
    assert "error" in response.json()


def test_register_rejects_malformed_json(api):
    response = api.post("/auth/register", data="{not json")

    assert response.status_code == 400


def test_login_returns_the_registered_user(api):
    user = api.register("alice")

    response = api.post("/auth/login", json={"username": "alice", "password": DEFAULT_PASSWORD})

    assert response.status_code == 200
    assert response.json()["userId"] == user["userId"]


def test_login_token_opens_protected_routes(api):
    api.register("alice")
    token = api.post("/auth/login", json={"username": "alice", "password": DEFAULT_PASSWORD}).json()["token"]

    assert api.get("/chats", token=token).status_code == 200


@pytest.mark.parametrize(
    "username, password",
    [("alice", "wrong-password"), ("nobody", DEFAULT_PASSWORD)],
    ids=["wrong password", "unknown user"],
)
def test_login_gives_the_same_error_for_bad_credentials(api, username, password):
    api.register("alice")

    response = api.post("/auth/login", json={"username": username, "password": password})

    assert response.status_code == 401
    assert response.json() == {"error": "Invalid username or password"}


@pytest.mark.parametrize(
    "headers",
    [{}, {"Authorization": "Bearer not-a-real-token"}, {"Authorization": "Basic abc"}],
    ids=["no header", "invalid token", "wrong scheme"],
)
def test_protected_route_rejects_bad_authorization(api, headers):
    response = api.get("/chats", headers=headers)

    assert response.status_code == 401
