import zmq

class Server(object):
    def __init__(self, context, endpoint, type):
        self.socket = context.socket(type)
        self.socket.bind(endpoint)

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


# Module-level polling function
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
    
    # Create poller
    poller = zmq.Poller()
    
    # Register all server sockets
    for server in servers:
        poller.register(server.socket, zmq.POLLIN)
    
    # Poll with timeout
    sockets = dict(poller.poll(timeout_ms))
    
    # Serve requests on sockets that are ready
    for server in servers:
        if server.socket in sockets and sockets[server.socket] == zmq.POLLIN:
            server.serve_once()
