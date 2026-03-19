#!/usr/bin/env python3
"""
media-server UDS 客户端测试脚本

支持两种握手协议：
  1) libuds 命令包（与 video-server 客户端一致）
  2) legacy 1 字节类型

用法：
  python3 test.py                        # 默认: libuds 协议请求 video
  python3 test.py video                  # libuds 协议请求 video
  python3 test.py audio                  # libuds 协议请求 audio
  python3 test.py audio_mic              # libuds 协议请求 audio_mic
  python3 test.py --legacy video         # legacy 1 字节协议请求 video
  python3 test.py --legacy audio         # legacy 1 字节协议请求 audio
"""

import sys
import struct
import json
import socket
import time

UDS_PATH = "/tmp/media_server.sock"

# ---------- 命令码定义（与 udsCmdDef.h 一致）----------
UDS_CMD_VIDEOSERVER_DEF      = 0x2000
VIDEOSERVER_CMD_ASSIGNVIDEO  = UDS_CMD_VIDEOSERVER_DEF + 1   # 0x2001
VIDEOSERVER_CMD_ASSIGNAUDIO  = UDS_CMD_VIDEOSERVER_DEF + 2   # 0x2002
VIDEOSERVER_CMD_ASSIGNAUDIO_MIC = UDS_CMD_VIDEOSERVER_DEF + 18  # 0x2012
VIDEOSERVER_CMD_GETIFRAME    = UDS_CMD_VIDEOSERVER_DEF + 5   # 0x2005
VIDEOSERVER_CMD_GETRESOLUTION = UDS_CMD_VIDEOSERVER_DEF + 4  # 0x2004
VIDEOSERVER_CMD_STOPVIDEO    = UDS_CMD_VIDEOSERVER_DEF + 21  # 0x2015
VIDEOSERVER_CMD_GETFRAMEINFO = UDS_CMD_VIDEOSERVER_DEF + 22  # 0x2016
VIDEOSERVER_CMD_SETFRAMEINFO = UDS_CMD_VIDEOSERVER_DEF + 23  # 0x2017

UDS_MARK = b'\xaa\xbb\xcc\xdd'
MAX_PACKS_NUM = 10

CMD_MAP = {
    "video":     VIDEOSERVER_CMD_ASSIGNVIDEO,
    "audio":     VIDEOSERVER_CMD_ASSIGNAUDIO,
    "audio_mic": VIDEOSERVER_CMD_ASSIGNAUDIO_MIC,
}


def build_uds_pack(cmd: int, payload: bytes) -> bytes:
    """
    构造 libuds 协议包: UdsHead + payload

    UdsHead 布局 (C struct, native byte order):
      unsigned char mark[4]          = {0xaa, 0xbb, 0xcc, 0xdd}
      int           cmd
      int           pack_count
      uint32_t      size[MAX_PACKS_NUM]
      int           crc
    """
    pack_count = 1
    sizes = [len(payload)] + [0] * (MAX_PACKS_NUM - 1)
    crc = 1011

    head = UDS_MARK
    head += struct.pack('<i', cmd)
    head += struct.pack('<i', pack_count)
    for s in sizes:
        head += struct.pack('<I', s)
    head += struct.pack('<i', crc)

    return head + payload


def send_libuds_request(sock: socket.socket, media: str):
    cmd = CMD_MAP[media]
    payload_obj = {"cmd": cmd, "msg": "media-server test client"}
    payload = json.dumps(payload_obj).encode('utf-8')
    pack = build_uds_pack(cmd, payload)
    sock.sendall(pack)
    print(f"[libuds] sent cmd=0x{cmd:04x} ({media}), payload={payload_obj}")


def send_legacy_request(sock: socket.socket, media: str):
    if media in ("video",):
        sock.sendall(b'v')
        print("[legacy] sent 'v' (video)")
    else:
        sock.sendall(b'a')
        print("[legacy] sent 'a' (audio)")


def recv_loop(sock: socket.socket):
    total = 0
    t0 = time.time()
    try:
        while True:
            data = sock.recv(65536)
            if not data:
                print("server closed connection")
                break
            total += len(data)
            elapsed = time.time() - t0
            print(f"recv {len(data)} bytes (total={total}, elapsed={elapsed:.1f}s)")
    except KeyboardInterrupt:
        print(f"\ninterrupted, total received: {total} bytes")
    except BrokenPipeError:
        print("connection broken")


def main():
    args = sys.argv[1:]
    legacy = False
    if "--legacy" in args:
        args.remove("--legacy")
        legacy = True

    media = args[0] if args else "video"

    if media not in CMD_MAP:
        print(f"unknown media type: {media}")
        print(f"valid types: {', '.join(CMD_MAP.keys())}")
        sys.exit(1)

    if legacy and media not in ("video", "audio"):
        print("legacy mode only supports 'video' or 'audio'")
        sys.exit(1)

    sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
    try:
        sock.connect(UDS_PATH)
        print(f"connected to {UDS_PATH}")
    except ConnectionRefusedError:
        print(f"connection refused: {UDS_PATH} (is media-server running?)")
        sys.exit(1)
    except FileNotFoundError:
        print(f"socket not found: {UDS_PATH} (is media-server running?)")
        sys.exit(1)

    if legacy:
        send_legacy_request(sock, media)
    else:
        send_libuds_request(sock, media)

    print("waiting for stream data... (Ctrl+C to stop)\n")
    recv_loop(sock)
    sock.close()


if __name__ == "__main__":
    main()
