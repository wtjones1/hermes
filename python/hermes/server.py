import zmq

class Server(object):
    def __init__(self, context, endpoint, type):
        self.socket = context.socket(type)
        self.socket.bind(endpoint)
        self.socket_type = type
        self.client_identity = None
    
    def is_router(self):
        return self.socket_type == zmq.ROUTER
    
    def receive_identity(self):
        if not self.is_router():
            return
        
        self.client_identity = self.socket.recv()
        delimiter = self.socket.recv()
    
    def send_identity(self):
        if not self.is_router():
            return
        
        self.socket.send(self.client_identity, zmq.SNDMORE)
        self.socket.send(b'', zmq.SNDMORE)  # Empty delimiter
    
    def receive_request(self):
        self.receive_identity()
        return self.socket.recv()
    
    def send_reply(self, data, flags=0):
        self.send_identity()
        return self.socket.send(data, flags)
    
    def send_multipart(self, msg_parts):
        if self.is_router():
            # Prepend identity and delimiter
            full_msg = [self.client_identity, b''] + msg_parts
            return self.socket.send_multipart(full_msg)
        else:
            return self.socket.send_multipart(msg_parts)

    def close(self):
        self.socket.close()
        self.socket = None

    def serve(self, count=None):
        if count is None:
            while True:
                self.serve_once()
        else:
            for n in range(count):
                self.serve_once()


def poll_and_serve(servers, timeout_ms=100):
    """
    Poll multiple servers and call serve_once on those with pending requests.
    
    Args:
        servers: List of Server instances to poll
        timeout_ms: Timeout in milliseconds:
                    -1 = block indefinitely
                     0 = return immediately (non-blocking)
                    >0 = wait up to timeout_ms milliseconds
    
    Returns when timeout expires or after serving all ready requests.
    """
    if not servers:
        return
    
    poller = zmq.Poller()
    
    for server in servers:
        poller.register(server.socket, zmq.POLLIN)
    
    sockets = dict(poller.poll(timeout_ms))
    
    for server in servers:
        if server.socket in sockets and sockets[server.socket] == zmq.POLLIN:
            server.serve_once()
