# godot_zeromq

ZeroMQ addon for Godot 4.4.

This is a fork of the `godot_zeromq` addon by [Fumiya Funatsu](https://github.com/funatsufumiya/godot_zeromq), 
adjusted to feel more like an integrated Godot component by using properties and 
signals instead of static functions. 

I don't provide a binary version yet, if you want this please ask in an issue.
You can compile this yourself though, or use the upstream fork instead, which has
[binary builds](https://github.com/funatsufumiya/godot_zeromq_bin) available.

## Install

Build the addon with `scons platform=linux` (or your desired platform), then
copy the `project/addons/zeromq/` to your project's addons directory.

Enable "ZeroMQ" in the project addons settings.

## Debug

For a debug build, use `scons target=template_debug debug_symbols=yes platform=...`. Now you can start 
godot or your exported project with a debugger and step into the library.

## Usage

Add a `ZMQSocket` node and configure its properties. Call `socket_node.start()`
from your script, or enable autostart to connect the socket when the scene is loaded.

You can connect to the `message_received` signal to process messages. The
signal argument is an array of [`PackedByteArray`](https://docs.godotengine.org/en/stable/classes/class_packedbytearray.html)
which are the frames of the ZMQ multipart message.

```gdscript
@onready var socket = $"../ZMQSocket"

func _ready():
    socket.message_received.connect(on_message_received)

func on_message_received(frames: Array[PackedByteArray]):
    print(frames)
```

To send something on the socket, build a multipart message of
`PackedByteArray`s and call `send_message`:

```gdscript
func send_something():
    var bytes: PackedByteArray = [13, 37]
    socket.send_message([bytes])
```

For a REQ socket, it makes sense to use the `await <signal>` mechanism of GDScript:

```gdscript
socket.send_message(request_frames)
response_frames = await socket.message_received
```

Make sure not to overlap message pairs in REQ/RES modes. Usually, DEALER/ROUTER
or PUB/SUB are better in those cases.
