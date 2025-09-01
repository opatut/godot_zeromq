#include "zmq_socket.hpp"

#include <iostream>
#include <zmq.h>

#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/typed_array.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

// #define PRINT(x) UtilityFunctions::print(x);
#define PRINT(x) std::cout << x << std::endl;

void ZMQSocket::_bind_methods() {
    ClassDB::bind_method(D_METHOD("start"), &ZMQSocket::start);
    ClassDB::bind_method(D_METHOD("stop"), &ZMQSocket::stop);
    ClassDB::bind_method(D_METHOD("send_message", "frames"), &ZMQSocket::send_message);

    ClassDB::bind_method(D_METHOD("get_address"), &ZMQSocket::get_address);
    ClassDB::bind_method(D_METHOD("set_address", "address"), &ZMQSocket::set_address);
    ClassDB::bind_method(D_METHOD("get_socket_type"), &ZMQSocket::get_socket_type);
    ClassDB::bind_method(D_METHOD("set_socket_type", "socket_type"), &ZMQSocket::set_socket_type);
    ClassDB::bind_method(D_METHOD("get_connection_mode"), &ZMQSocket::get_connection_mode);
    ClassDB::bind_method(D_METHOD("set_connection_mode", "connection_mode"), &ZMQSocket::set_connection_mode);
    ClassDB::bind_method(D_METHOD("get_autostart"), &ZMQSocket::get_autostart);
    ClassDB::bind_method(D_METHOD("set_autostart", "autostart"), &ZMQSocket::set_autostart);
    ClassDB::bind_method(D_METHOD("get_sub_filter"), &ZMQSocket::get_sub_filter);
    ClassDB::bind_method(D_METHOD("set_sub_filter", "sub_filter"), &ZMQSocket::set_sub_filter);

    ADD_PROPERTY(PropertyInfo(Variant::STRING, "address"), "set_address", "get_address");
    ADD_PROPERTY(
        PropertyInfo(
            Variant::INT, "socket_type", PROPERTY_HINT_ENUM, "PUB,SUB,REQ,REP,DEALER,ROUTER,PULL,PUSH,XPUB,XSUB,STREAM"
        ),
        "set_socket_type", "get_socket_type"
    );
    ADD_PROPERTY(
        PropertyInfo(Variant::INT, "connection_mode", PROPERTY_HINT_ENUM, "Bind,Connect"), "set_connection_mode",
        "get_connection_mode"
    );
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "autostart"), "set_autostart", "get_autostart");
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "sub_filter"), "set_sub_filter", "get_sub_filter");

    BIND_ENUM_CONSTANT(ZMQ_SOCKET_TYPE_PUB);
    BIND_ENUM_CONSTANT(ZMQ_SOCKET_TYPE_SUB);
    BIND_ENUM_CONSTANT(ZMQ_SOCKET_TYPE_REQ);
    BIND_ENUM_CONSTANT(ZMQ_SOCKET_TYPE_REP);
    BIND_ENUM_CONSTANT(ZMQ_SOCKET_TYPE_DEALER);
    BIND_ENUM_CONSTANT(ZMQ_SOCKET_TYPE_ROUTER);
    BIND_ENUM_CONSTANT(ZMQ_SOCKET_TYPE_PULL);
    BIND_ENUM_CONSTANT(ZMQ_SOCKET_TYPE_PUSH);
    BIND_ENUM_CONSTANT(ZMQ_SOCKET_TYPE_XPUB);
    BIND_ENUM_CONSTANT(ZMQ_SOCKET_TYPE_XSUB);
    BIND_ENUM_CONSTANT(ZMQ_SOCKET_TYPE_STREAM);

    BIND_ENUM_CONSTANT(ZMQ_CONNECTION_MODE_BIND);
    BIND_ENUM_CONSTANT(ZMQ_CONNECTION_MODE_CONNECT);

    // ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "speed", PROPERTY_HINT_RANGE, "0,20,0.01"), "set_speed", "get_speed");
    ADD_SIGNAL(MethodInfo("message_received", PropertyInfo(Variant::ARRAY, "message")));
}

ZMQSocket::ZMQSocket() {}
ZMQSocket::~ZMQSocket() { stop(); }

int _get_zmq_socket_type(ZMQSocket::SocketType socket_type) {
    switch (socket_type) {
    case ZMQSocket::ZMQ_SOCKET_TYPE_PUB:
        return ZMQ_PUB;
    case ZMQSocket::ZMQ_SOCKET_TYPE_SUB:
        return ZMQ_SUB;
    case ZMQSocket::ZMQ_SOCKET_TYPE_REQ:
        return ZMQ_REQ;
    case ZMQSocket::ZMQ_SOCKET_TYPE_REP:
        return ZMQ_REP;
    case ZMQSocket::ZMQ_SOCKET_TYPE_DEALER:
        return ZMQ_DEALER;
    case ZMQSocket::ZMQ_SOCKET_TYPE_ROUTER:
        return ZMQ_ROUTER;
    case ZMQSocket::ZMQ_SOCKET_TYPE_PULL:
        return ZMQ_PULL;
    case ZMQSocket::ZMQ_SOCKET_TYPE_PUSH:
        return ZMQ_PUSH;
    case ZMQSocket::ZMQ_SOCKET_TYPE_XPUB:
        return ZMQ_XPUB;
    case ZMQSocket::ZMQ_SOCKET_TYPE_XSUB:
        return ZMQ_XSUB;
    case ZMQSocket::ZMQ_SOCKET_TYPE_STREAM:
        return ZMQ_STREAM;

    default:
        return ZMQ_PUB;
    }
}

void ZMQSocket::_print_error(String where) {
    String what;
    switch (zmq_errno()) {
    case EAGAIN:
        what = "EAGAIN";
        break;
    case ENOTSUP:
        what = "ENOTSUP";
        break;
    case EFSM:
        what = "EFSM";
        break;
    case ETERM:
        what = "ETERM";
        break;
    case ENOTSOCK:
        what = "ENOTSOCK";
        break;
    case EINTR:
        what = "EINTR";
        break;
    case EINVAL:
        what = "EINVAL";
        break;
    case EPROTONOSUPPORT:
        what = "EPROTONOSUPPORT";
        break;
    case ENOCOMPATPROTO:
        what = "ENOCOMPATPROTO";
        break;
    case EMTHREAD:
        what = "EMTHREAD";
        break;
    default:
        what = "unknown";
    }

    std::cout << "ZMQ error in " << where.utf8().get_data() << ": " << what.utf8().get_data() << std::endl;
    UtilityFunctions::push_error(String("ZMQ Error in ") + where + String(": ") + what);
}

void ZMQSocket::_close_socket() {
    if (_socket != nullptr) {
        if (zmq_close(_socket) != 0) {
            _print_error("zmq_close");
        }
        _socket = nullptr;
    }

    if (_context != nullptr) {
        if (zmq_ctx_term(_context) != 0) {
            _print_error("zmq_ctx_term");
        }
        _context = nullptr;
    }
}

void ZMQSocket::start() {
    PRINT("start()");
    if (Engine::get_singleton()->is_editor_hint()) {
        UtilityFunctions::push_error("Cannot start ZMQ socket in editor.");
        return;
    }

    if (_address.is_empty()) {
        PRINT("No address");
        return;
    }

    _context = zmq_ctx_new();
    if (_context == NULL) {
        _print_error("zmq_ctx_new");
        return;
    }

    if (zmq_ctx_set(_context, ZMQ_IO_THREADS, 1) != 0) {
        _print_error("zmq_ctx_set");
        return;
    }

    _socket = zmq_socket(_context, _get_zmq_socket_type(_socket_type));
    if (_socket == NULL) {
        _print_error("zmq_socket");
        zmq_ctx_destroy(_context);
        _context = nullptr;
        return;
    }

    switch (_connection_mode) {
    case ZMQ_CONNECTION_MODE_BIND:
        if (zmq_bind(_socket, _address.utf8().get_data()) != 0) {
            _print_error("zmq_bind");
            _close_socket();
            return;
        }
        break;
    case ZMQ_CONNECTION_MODE_CONNECT:
        if (zmq_connect(_socket, _address.utf8().get_data()) != 0) {
            _print_error("zmq_connect");
            _close_socket();
            return;
        }
        break;
    default:
        UtilityFunctions::push_error("ZMQSocket::init unknown connection mode: " + String::num_int64(_connection_mode));
        return;
    }

    if (_socket_type == ZMQ_SOCKET_TYPE_SUB) {
        auto sub_filter = _sub_filter.utf8();
        if (zmq_setsockopt(_socket, ZMQ_SUBSCRIBE, sub_filter.get_data(), sub_filter.length()) != 0) {
            _print_error("zmq_setsockopt");
            return;
        }
        PRINT("Subscribed");
        PRINT(sub_filter.get_data());
    }

    if (_socket_type != ZMQ_SOCKET_TYPE_REQ) {
        _should_receive = true;
    }
}

bool ZMQSocket::stop() {
    if (_socket != nullptr) {
        _close_socket();
        _should_receive = false;
        return true;
    }
    return false;
}

void ZMQSocket::send_message(TypedArray<PackedByteArray> frames) {
    if (_should_receive && (_socket_type == ZMQ_SOCKET_TYPE_REQ || _socket_type == ZMQ_SOCKET_TYPE_REP)) {
        UtilityFunctions::push_error(
            "Socket cannot send a message now, as it is waiting for a message from peer (this is REQ-REP behaviour)."
        );
        return;
    }

    int n = frames.size();
    for (int i = 0; i < n; i++) {
        const PackedByteArray &message = frames[i];

        if (zmq_send(_socket, message.ptr(), message.size(), i < n - 1 ? ZMQ_SNDMORE : 0) < 0) {
            _print_error("zmq_send");
            return;
        }
    }

    // In single-recv modes (REQ, REP), we now want to find a new message.
    if (_socket_type == ZMQ_SOCKET_TYPE_REQ || _socket_type == ZMQ_SOCKET_TYPE_REP) {
        _should_receive = true;
    }
}

// Property access methods
String ZMQSocket::get_address() const { return _address; }
void ZMQSocket::set_address(String address) {
    bool stopped = stop();
    _address = address;
    if (stopped) {
        start();
    }
}

ZMQSocket::SocketType ZMQSocket::get_socket_type() const { return _socket_type; }
void ZMQSocket::set_socket_type(ZMQSocket::SocketType socket_type) {
    bool stopped = stop();
    _socket_type = socket_type;
    if (stopped) {
        start();
    }
}

ZMQSocket::ConnectionMode ZMQSocket::get_connection_mode() const { return _connection_mode; }
void ZMQSocket::set_connection_mode(ZMQSocket::ConnectionMode connection_mode) {
    bool stopped = stop();
    _connection_mode = connection_mode;
    if (stopped) {
        start();
    }
}

bool ZMQSocket::get_autostart() const { return _autostart; }
void ZMQSocket::set_autostart(bool autostart) { _autostart = autostart; }

String ZMQSocket::get_sub_filter() const { return _sub_filter; }
void ZMQSocket::set_sub_filter(String sub_filter) {
    _sub_filter = sub_filter;

    if (_socket != nullptr) {
        // Change the filter
        auto utf8 = _sub_filter.utf8();
        if (zmq_setsockopt(_socket, ZMQ_SUBSCRIBE, utf8, utf8.length()) != 0) {
            _print_error("zmq_setsockopt");
            return;
        }
    }
}

void ZMQSocket::_ready() {
    if (_autostart && !Engine::get_singleton()->is_editor_hint()) {
        start();
    }
}

void ZMQSocket::_process(double delta) {
    if (_socket != nullptr && !Engine::get_singleton()->is_editor_hint()) {
        while (_should_receive) {
            if (!_recv_single()) {
                // We did not get a message, retry next process_frame.
                break;
            }

            // We got a message. If we're in a single-recv mode (REQ, REP), stop
            // receiving until we send the next message.
            if (_socket_type == ZMQ_SOCKET_TYPE_REQ || _socket_type == ZMQ_SOCKET_TYPE_REP) {
                _should_receive = false;
            }
        }
    }
}

bool ZMQSocket::_recv_single() {
    zmq_msg_t msg;

    TypedArray<PackedByteArray> frames;
    while (true) {
        if (zmq_msg_init(&msg) != 0) {
            _print_error("zmq_msg_init");
            goto error;
        }

        if (zmq_msg_recv(&msg, _socket, ZMQ_DONTWAIT) == -1) {
            if (zmq_errno() == EAGAIN) {
                // No message available
                goto error;
            }

            _print_error("zmq_msg_recv");
            goto error;
        }

        // Copy message frame into a PackedByteArray
        size_t size = zmq_msg_size(&msg);
        PackedByteArray frame;
        frame.resize(size);
        memcpy(frame.ptrw(), zmq_msg_data(&msg), size);
        frames.push_back(frame);

        // Free the message
        if (zmq_msg_close(&msg) != 0) {
            _print_error("zmq_msg_close");
            goto error;
        }

        if (!zmq_msg_more(&msg)) {
            break;
        }
    }
    emit_signal("message_received", frames);
    return true;

error:
    zmq_msg_close(&msg);
    return false;
}
