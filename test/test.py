import socket

sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
sock.connect("/tmp/media_server.sock")
sock.send(bytes([0]))  # 选择 0: video,  1: audio

while True:
    data = sock.recv(4096)
    if not data:
        break
    print("recv", len(data))