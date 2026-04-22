#include "hermes/server.hpp"
#include <cstring>

namespace hermes {

server::server(void* a_context, const std::string& a_endpoint, int a_type)
{
  m_socket = zmq_socket(a_context, a_type);
  int code = zmq_bind(m_socket, a_endpoint.c_str());
  if (code < 0)
  {
    throw error(zmq_strerror(errno));
  }
}

server::~server()
{
  if (m_socket)
  {
    zmq_close(m_socket);
  }
}

void
server::close()
{
  if (m_socket)
  {
    int code = zmq_close(m_socket);
    if (code < 0)
    {
      throw error(zmq_strerror(errno));
    }
    m_socket = nullptr;
  }
}

void
server::serve()
{
  while (true)
  {
    serve_once();
  }
}

void
server::serve(int a_count)
{
  for (auto n = 0; n < a_count; ++n)
  {
    serve_once();
  }
}

// Standalone polling function implementation
void
poll_and_serve(std::vector<server*>& servers, int timeout_ms)
{
  if (servers.empty())
  {
    return;
  }
  
  // Prepare poll items
  std::vector<zmq_pollitem_t> poll_items(servers.size());
  
  for (size_t i = 0; i < servers.size(); ++i)
  {
    poll_items[i].socket = servers[i]->m_socket;
    poll_items[i].fd = 0;
    poll_items[i].events = ZMQ_POLLIN;
    poll_items[i].revents = 0;
  }
  
  // Poll all sockets
  int rc = zmq_poll(poll_items.data(), static_cast<int>(poll_items.size()), 
                    timeout_ms);
  
  if (rc < 0)
  {
    // Error occurred
    throw error(zmq_strerror(errno));
  }
  
  if (rc == 0)
  {
    // Timeout - no messages
    return;
  }
  
  // Serve requests on sockets that are ready
  for (size_t i = 0; i < servers.size(); ++i)
  {
    if (poll_items[i].revents & ZMQ_POLLIN)
    {
      servers[i]->serve_once();
    }
  }
}

} // hermes namespace
