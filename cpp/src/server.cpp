#include "hermes/server.hpp"
#include <cstring>

namespace hermes {

server::server(void* a_context, const std::string& a_endpoint, int a_type)
  : m_socket_type(a_type)
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
  if (is_router() && !m_client_identity.empty())
  {
    m_client_identity.clear();
  }

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
server::receive_identity()
{
  if (!is_router()) return;
  
  zmq_msg_t identity;
  zmq_msg_init(&identity);
  zmq_msg_recv(&identity, m_socket, 0);
  
  size_t size = zmq_msg_size(&identity);
  m_client_identity.resize(size);
  memcpy(m_client_identity.data(), zmq_msg_data(&identity), size);
  zmq_msg_close(&identity);
  
  zmq_msg_t delimiter;
  zmq_msg_init(&delimiter);
  zmq_msg_recv(&delimiter, m_socket, 0);
  zmq_msg_close(&delimiter);
}

void
server::send_identity()
{
  if (!is_router()) return;
  
  zmq_msg_t identity;
  zmq_msg_init_size(&identity, m_client_identity.size());
  memcpy(zmq_msg_data(&identity), m_client_identity.data(), 
         m_client_identity.size());
  zmq_msg_send(&identity, m_socket, ZMQ_SNDMORE);
  
  zmq_msg_t delimiter;
  zmq_msg_init_size(&delimiter, 0);
  zmq_msg_send(&delimiter, m_socket, ZMQ_SNDMORE);
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

void
poll_and_serve(std::vector<server*>& servers, int timeout_ms)
{
  if (servers.empty())
  {
    return;
  }
  
  std::vector<zmq_pollitem_t> poll_items(servers.size());
  
  for (size_t i = 0; i < servers.size(); ++i)
  {
    poll_items[i].socket = servers[i]->m_socket;
    poll_items[i].fd = 0;
    poll_items[i].events = ZMQ_POLLIN;
    poll_items[i].revents = 0;
  }
  
  int rc = zmq_poll(poll_items.data(), static_cast<int>(poll_items.size()), 
                    timeout_ms);
  
  if (rc < 0)
  {
    throw error(zmq_strerror(errno));
  }
  
  if (rc == 0)
  {
    return;
  }
  
  for (size_t i = 0; i < servers.size(); ++i)
  {
    if (poll_items[i].revents & ZMQ_POLLIN)
    {
      servers[i]->serve_once();
    }
  }
}

} // hermes namespace
