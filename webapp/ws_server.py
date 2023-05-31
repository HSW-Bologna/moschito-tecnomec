#!/usr/bin/env python3
import asyncio
import websockets

PORT = 8765


async def server(websocket):
    async for message in websocket:
        print(f"Received:\t{message}")
        message_bytes = bytearray(message)
        message_bytes[-1] ^= 1
        print("Waiting........")
        await asyncio.sleep(3)
        await websocket.send(bytes(message_bytes))
        print(f"Sent:\t\t{bytes(message_bytes)}\n")


async def main():
    print(f"Listening on ws://localhost:{PORT}\n")
    async with websockets.serve(server, "localhost", PORT):
        await asyncio.Future()  # run forever


asyncio.run(main())
