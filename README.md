# godot_zeromq

ZeroMQ addon for Godot 4.2.2 - 4.4

## Install

use [godot_zeromq_bin](https://github.com/funatsufumiya/godot_zeromq_bin). see [instruction](https://github.com/funatsufumiya/godot_zeromq_bin/blob/main/README.md)

## Usage

```gdscript
@onready var zmq_receiver = ZMQReceiver.new_from("tcp://localhost:5555", ZMQ.SocketType.PULL, ZMQ.ConnectionMode.CONNECT, "")
@onready var zmq_sender = ZMQSender.new_from("tcp://localhost:5555", ZMQ.SocketType.PUSH, ZMQ.ConnectionMode.BIND, "", false)

func _ready():
    add_child(zmq_receiver)
    add_child(zmq_sender)

    zmq_receiver.onMessageString(func(str: String):
        print("[ZMQ Receiver] Received: ", str)
    )

    while true:
        await get_tree().create_timer(1.0).timeout
        print("[ZMQ Sender] Sending: ", "Hello World")
        zmq_sender.sendString("Hello World")

func _exit_tree():
    zmq_receiver.stop()
    zmq_sender.stop()

    remove_child(zmq_receiver)
    remove_child(zmq_sender)
```

more example, see [GDScript of demo project](https://github.com/funatsufumiya/godot_zeromq/blob/main/project/zeromq_receiver.gd)

### ZMQReceiver

- `new_from(address: String, socket_type: int, connection_mode: int, socket_filter: String) -> ZMQReceiver`
- `onMessageString(callback: Callable[[String], void])`
- `onMessageBytes(callback: Callable[[PackedByteArray], void])`
- `sendString(message: String) -> void`
- `sendBytes(message: PackedByteArray) -> void`
- `stop() -> void`

### ZMQSender

- `new_from(address: String, socket_type: int, connection_mode: int, socket_filter: String, auto_receive_on_sender: bool) -> ZMQSender`
- `onMessageString(callback: Callable[[String], void])` (***ONLY enabled to use when auto_receive_on_sender is true, or `beginReceiveRequest()` called***)
- `onMessageBytes(callback: Callable[[PackedByteArray], void])` (***ONLY enabled to use when auto_receive_on_sender is true, or `beginReceiveRequest()` called***)
- `sendString(message: String) -> void`
- `sendBytes(message: PackedByteArray) -> void`
- `beginReceiveRequest() -> void` (***ONLY enabled to use when auto_receive_on_sender is false***)
- `stop() -> void`

### NOTES of ZMQReceiver and ZMQSender

- `socket_filter` is ONLY used when `socket_type` is `SUB`
- `onMessageString` and `onMessageBytes` are exclusive, you can ONLY use one of them

### ZMQ

```gdscript
enum SocketType {
    PUB = 1,
    SUB = 2,
    REQ = 3,
    REP = 4,
    DEALER = 5,
    ROUTER = 6,
    PULL = 7,
    PUSH = 8,
    XPUB = 9,
    XSUB = 10,
    STREAM = 11
}

enum ConnectionMode {
    BIND = 1,
    CONNECT = 2
}
```

## Features

- SUB/PUB, PUSH/PULL, REQ/REP (tested)
- untested but enabled: DEALER/ROUTER, PAIR/PAIR, XPUB/XSUB, STREAM

## Example Project

see [`project/`](project) directory

## Build and Run

(This process is needed only if you build this plugin by your own)

- NOTE: before scons, you need to install **cppzmq** and **ZeroMQ**. 
  - on Ubuntu/Mac: `brew install cppzmq` (installed dependency will auto detected from `/home/linuxbrew/.linuxbrew/`, and you can also use vcpkg if you want, path should be `~/vcpkg/`)
  - on Windows: `./vcpkg install cppzmq` (installed dependency will auto detected from `C:/vcpkg/`)

```bash
$ git submodule update --init --recursive --recommend-shallow --depth 1
$ scons
$ scons target=template_release
$ godot project/project.godot # (only first time)
$ godot --path project/ # run demo
```
