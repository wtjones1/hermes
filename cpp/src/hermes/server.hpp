#ifndef HERMES_SERVER_HPP
#define HERMES_SERVER_HPP

#include <string>
#include <vector>
#include <zmq.h>
#include "hermes/error.hpp"

namespace hermes {

class server
{
public:
  server(void* a_context, const std::string& a_endpoint, int a_type);
  virtual ~server();

  void close();
  void serve();
  void serve(int a_count);
  virtual void serve_once() = 0;
protected:
  void* m_socket;

  friend void poll_and_serve(std::vector<server*>& servers, int timeout_ms);
};

void poll_and_serve(std::vector<server*>& servers, int timeout_ms);

} // hermes namesapce

#endif // HERMES_SERVER_HPP
