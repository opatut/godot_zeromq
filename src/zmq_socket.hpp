#pragma once

#include <godot_cpp/classes/mutex.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/thread.hpp>

#include <memory>

namespace godot {
class ZMQSocket : public Node {
    GDCLASS(ZMQSocket, Node);

  public:
    enum SocketType {
        ZMQ_SOCKET_TYPE_PUB,
        ZMQ_SOCKET_TYPE_SUB,
        ZMQ_SOCKET_TYPE_REQ,
        ZMQ_SOCKET_TYPE_REP,
        ZMQ_SOCKET_TYPE_DEALER,
        ZMQ_SOCKET_TYPE_ROUTER,
        ZMQ_SOCKET_TYPE_PULL,
        ZMQ_SOCKET_TYPE_PUSH,
        ZMQ_SOCKET_TYPE_XPUB,
        ZMQ_SOCKET_TYPE_XSUB,
        ZMQ_SOCKET_TYPE_STREAM,
    };
    enum ConnectionMode { ZMQ_CONNECTION_MODE_BIND, ZMQ_CONNECTION_MODE_CONNECT };

    ZMQSocket();
    ~ZMQSocket();

    // Lifecycle methods
    void _ready() override;
    void _process(double delta) override;
    bool _recv_single();

    // Public methods to be called by consumer or automatically on property change
    void start();
    bool stop();
    void send_message(TypedArray<PackedByteArray> frames);

    // Property access methods
    String get_address() const;
    void set_address(String address);

    SocketType get_socket_type() const;
    void set_socket_type(SocketType socket_type);

    ConnectionMode get_connection_mode() const;
    void set_connection_mode(ConnectionMode connection_mode);

    bool get_autostart() const;
    void set_autostart(bool autostart);

    String get_sub_filter() const;
    void set_sub_filter(String sub_filter);

  protected:
    static void _bind_methods();

  private:
    void _print_error(String where);
    void _close_socket();

    void *_context = nullptr;
    void *_socket = nullptr;

    // Proprties
    String _address;
    SocketType _socket_type;
    ConnectionMode _connection_mode;
    bool _autostart;
    String _sub_filter;

    // Set to true when the process loop should try to receive messages.
    bool _should_receive = false;
};

} // namespace godot

VARIANT_ENUM_CAST(godot::ZMQSocket::SocketType);
VARIANT_ENUM_CAST(godot::ZMQSocket::ConnectionMode);
