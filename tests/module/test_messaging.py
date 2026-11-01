"""The main workflow of the messenger, from creating a chat to deleting it, against the real server."""


def test_two_users_exchange_a_message_and_delete_the_chat(api):
    alice = api.register("alice")
    bob = api.register("bob")
    carol = api.register("carol")

    # Alice creates the chat and sends a message.
    created = api.post("/chats", token=alice["token"], json={"username": "bob"})
    assert created.status_code == 200
    chat_id = created.json()["id"]

    sent = api.post(f"/chats/{chat_id}/messages", token=alice["token"], json={"text": "hi bob"})
    assert sent.status_code == 201
    assert sent.json()["status"] == "sent"
    assert sent.json()["senderId"] == alice["userId"]

    # Bob sees it as unread.
    summary = api.get("/chats", token=bob["token"]).json()[0]
    assert summary["chatId"] == chat_id
    assert summary["partnerUsername"] == "alice"
    assert summary["unreadCount"] == 1
    assert summary["latestMessage"]["text"] == "hi bob"

    # Alice opening her own chat doesn't mark her message as read.
    assert api.get(f"/chats/{chat_id}", token=alice["token"]).json()["messages"][0]["status"] == "sent"

    # A third user can't reach the chat in any way.
    assert api.get("/chats", token=carol["token"]).json() == []
    assert api.get(f"/chats/{chat_id}", token=carol["token"]).status_code == 404
    assert api.post(f"/chats/{chat_id}/messages", token=carol["token"], json={"text": "hi"}).status_code == 404
    assert api.delete(f"/chats/{chat_id}", token=carol["token"]).status_code == 404

    # Bob opening the chat marks the message as read, for both users.
    view = api.get(f"/chats/{chat_id}", token=bob["token"]).json()
    assert [m["text"] for m in view["messages"]] == ["hi bob"]
    assert api.get("/chats", token=bob["token"]).json()[0]["unreadCount"] == 0
    assert api.get(f"/chats/{chat_id}", token=alice["token"]).json()["messages"][0]["status"] == "read"

    # Deleting removes the chat for both users.
    assert api.delete(f"/chats/{chat_id}", token=alice["token"]).status_code == 204
    assert api.get("/chats", token=alice["token"]).json() == []
    assert api.get("/chats", token=bob["token"]).json() == []
    assert api.get(f"/chats/{chat_id}", token=bob["token"]).status_code == 404
