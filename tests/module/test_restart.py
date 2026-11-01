"""Tokens from before a restart must not work afterwards, as user IDs start again from 1."""


def test_old_token_cannot_read_the_chat_of_a_new_user_with_the_same_id(server, api):
    old = api.register("old-user")

    server.restart()

    new = api.register("new-user")
    partner = api.register("partner")
    assert new["userId"] == old["userId"]
    chat_id = api.post("/chats", token=new["token"], json={"username": "partner"}).json()["id"]
    api.post(f"/chats/{chat_id}/messages", token=partner["token"], json={"text": "private"})

    assert api.get("/chats", token=old["token"]).status_code == 401
    assert api.get(f"/chats/{chat_id}", token=old["token"]).status_code == 401
    assert api.get(f"/chats/{chat_id}", token=new["token"]).status_code == 200
