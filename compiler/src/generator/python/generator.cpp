#include "generator/python/generator.hpp"

namespace hermes {
namespace compiler {
namespace generator {
namespace python {

generator::generator()
  : m_blueprint(nullptr)
{
  // empty
}

void
generator::open(const std::string& a_project, const std::string& a_directory)
{
  m_project = a_project;
  m_py_path = a_directory + "/" + a_project + ".py";
  m_py.open(m_py_path);
  m_py << tabsize(4);

  m_stub_path = a_directory + "/" + a_project + "_stubs.py";
  m_stub.open(m_stub_path);
  m_stub << tabsize(4);
}

void
generator::write(const state::blueprint& a_blueprint)
{
  m_blueprint = &a_blueprint;
  write_header();
  write_structures();
  write_interfaces();
  write_stubs();
  m_blueprint = nullptr;

  // write_enumerations();
  // write_constants();
}

void
generator::close()
{
  m_project.clear();
  m_py_path.clear();
  m_py.close();
  m_stub_path.clear();
  m_stub.close();
}

void
generator::write_header()
{
  using std::endl;
  m_py << tab << "import hermes" << endl;
  m_py << tab << "import struct" << endl;
  m_py << tab << "import sys" << endl;
  for (auto import : m_blueprint->imports())
  {
    m_py << tab << "from " << stem(import.first) << " import *" << endl;
  }
  m_py << endl;
}

void
generator::write_structures()
{
  for (auto ptr : m_blueprint->structures())
  {
    write_structure(*ptr);
  }
  for (auto ptr : m_blueprint->exceptions())
  {
    write_exception(*ptr);
  }
}

void
generator::write_interfaces()
{
  for (const auto& i : m_blueprint->interfaces())
  {
    write_interface(i);
  }
}

void
generator::write_stubs()
{
  using namespace std::placeholders;
  auto ifaces = m_blueprint->interfaces();
  auto write_interface = std::bind(&generator::write_stub, this, _1);

  std::for_each(ifaces.begin(), ifaces.end(), write_interface);
}

void
generator::write_structure(const state::structure& a_structure)
{
  using std::endl;

  auto name = to_camel(a_structure.name());
  const auto& fields = a_structure.fields();

  m_py << tab << "class " << name << ":" << endl;
  m_py << indent;
  constructor(fields);
  reader(fields);
  writer(fields);
  stringer(fields);
  m_py << unindent;
}

void
generator::write_exception(const state::structure& a_exception)
{
  using std::endl;
  using std::ostream;
  typedef const state::field& field_t;

  auto name = a_exception.name();
  auto fields = a_exception.fields();
  auto member = [](ostream& out, field_t f){ out << f.name(); };
  auto args = join(fields.begin(), fields.end(), member);

  m_py << tab << "class " << name << "(Exception):" << endl;
  m_py << indent;
  m_py << tab << "def __init__(self, " << args << "):" << endl;
  m_py << indent;

  // Call Exception base constructor with first field (typically the message)
  if (!fields.empty())
  {
    m_py << tab << "super(" << name << ", self).__init__("
         << fields[0].name() << ")" << endl;
  }

  for (const auto& field_ : fields)
  {
    m_py << tab << "self." << field_.name() << " = " << field_.name() << endl;
  }
  m_py << unindent;
  m_py << endl;

  reader(fields);
  writer(fields);
  stringer(fields);
  m_py << unindent;
  m_py << endl;
}

void
generator::write_interface(const state::interface& a_interface)
{
  m_py << tab << "class " << to_camel(a_interface.name()) << ":" << std::endl;
  m_py << indent;
  client(a_interface);
  server(a_interface);
  m_py << unindent;
}

void
generator::write_stub(const state::interface& a_interface)
{
  using std::endl;

  auto name = to_camel(a_interface.name());
  auto procedures = a_interface.procedures();
  auto exceptions = a_interface.exceptions();

  m_stub << tab << "import " << m_project << endl;
  m_stub << endl;

  m_stub << tab << "class " << name << "Stub(" << m_project << ".";
  m_stub << name << ".Server):" << endl;
  m_stub << indent;

  m_stub << tab << "def __init__(self, context, endpoint, socket_type):" << endl;
  m_stub << indent;
  m_stub << tab << "super().__init__(context, endpoint, socket_type)" << endl;
  m_stub << unindent << endl;

  for (const auto& proc : procedures)
  {
    auto method_name = proc.name();
    auto result = proc.result();
    auto params = proc.parameters();
    auto is_void = result->is_void();

    m_stub << tab << "def " << method_name << "(self";
    for (const auto& p : params)
    {
      m_stub << ", " << p.name();
    }
    m_stub << "):" << endl;
    m_stub << indent;

    if (is_void)
    {
      m_stub << tab << "pass" << endl;
    }
    else
    {
      pointer result_type = translate(result);
      m_stub << tab << "result = " << result_type->default_value() << endl;
      m_stub << tab << "return result" << endl;
      m_stub << tab << endl;
    }

    if (!exceptions.empty())
    {
      m_stub << tab << "# You can raise exceptions if needed:" << endl;
      for (auto e : exceptions)
      {
        m_stub << tab << "# raise " << m_project << "." << e->name();
        m_stub << "(\"Error message\", error_code)" << endl;
      }
    }

    m_stub << unindent << endl;
  }

  m_stub << unindent << endl;
  m_stub << "if __name__ == \"__main__\":" << endl;
  m_stub << indent;
  m_stub << tab << "import zmq" << endl << endl;
  m_stub << tab << "# Create ZeroMQ context" << endl;
  m_stub << tab << "context = zmq.Context()" << endl << endl;
  m_stub << tab << "# Create server instance" << endl;
  m_stub << tab << "endpoint = \"tcp://*:5555\"  # Example endpoint" << endl;
  m_stub << tab << "socket_type = zmq.ROUTER    # Typical socket type for Hermes servers";
  m_stub << endl << endl;
  m_stub << tab << "server = " << name << "Stub(context, endpoint, socket_type)";
  m_stub << endl;
  m_stub << unindent;
}

void
generator::constructor(const field_vector& a_fields)
{
  using std::endl;
  using std::ostream;
  typedef const state::field& field_t;

  std::string name;
  auto argument = [](ostream& out, field_t f){
      pointer type = translate(f.type());
      out << f.name() << "=" << type->default_value();
    };

  m_py << tab << "def __init__(self";
  m_py << join(a_fields.begin(), a_fields.end(), argument).left(", ");
  m_py << "):" << endl;
  m_py << indent;
  for (const auto& field_ : a_fields)
  {
    name = field_.name();
    m_py << tab << "self." << name << " = " << name << endl;
  }
  m_py << unindent;
  m_py << endl;
}

void
generator::reader(const field_vector& a_fields)
{
  using std::endl;
  using std::ostream;
  typedef const state::field& field_t;

  std::string name;
  pointer type;

  auto member = [](ostream& out, field_t f){ out << f.name(); };
  auto ctor = join(a_fields.begin(), a_fields.end(), member).left("cls(").right(")");

  m_py << tab << "@classmethod" << std::endl;
  m_py << tab << "def read(cls, data, position):" << endl;
  m_py << indent;
  for (const auto& field_ : a_fields)
  {
    name = field_.name();
    type = translate(field_.type());
    type->unpack(m_py, name); // unpack() needs to update position
  }
  m_py << tab << "return (" << ctor << ", position)" << endl;
  m_py << unindent;
  m_py << endl;
}

void
generator::writer(const field_vector& a_fields)
{
  using std::endl;

  std::string name;
  pointer type;

  m_py << tab << "def write(self, buffer):" << endl;
  m_py << indent;
  for (const auto& field_ : a_fields)
  {
    name = "self." + field_.name();
    type = translate(field_.type());
    type->pack(m_py, name);  // This will append to buffer
  }
  m_py << unindent;
  m_py << endl;
}

void
generator::stringer(const field_vector& a_fields)
{
  using std::endl;
  using std::ostream;
  typedef const state::field& field_t;

  auto member = [](ostream& out, field_t f){ out << "self." << f.name(); };

  m_py << tab << "def __str__(self):" << endl;
  m_py << indent;
  m_py << tab << "return str(";
  m_py << join(a_fields.begin(), a_fields.end(), member).left("(").right(")");
  m_py << ")" << endl;
  m_py << unindent;
  m_py << endl;
}

void
generator::client(const state::interface& a_interface)
{
  using std::endl;

  auto header_id = 0;
  auto name = to_camel(a_interface.name());
  auto parent = "super(" + name + ".Client, self)";

  m_py << tab << "class Client(hermes.Client):" << endl;
  m_py << indent;
  m_py << tab << "def __init__(self, context, endpoint, type_):" << endl;
  m_py << indent;
  m_py << tab << parent << ".__init__(context, endpoint, type_)" << endl;
  m_py << endl;
  m_py << unindent;
  for (const auto& procedure_ : a_interface.procedures())
  {
    header_id++;
    client_method(procedure_, header_id);
  }
  m_py << unindent;
}

void
generator::server(const state::interface& a_interface)
{
  using std::endl;

  auto header_id = 0;
  auto name = to_camel(a_interface.name());
  auto parent = "super(" + name + ".Server, self)";
  auto error_message = "Received a request for an undefined procedure";

  m_py << tab << "class Server(hermes.Server):" << endl;
  m_py << indent;
  m_py << tab << "def __init__(self, context, endpoint, type_):" << endl;
  m_py << indent;
  m_py << tab << parent << ".__init__(context, endpoint, type_)" << endl;
  m_py << unindent;
  m_py << endl;
  m_py << tab << "def serve_once(self):" << endl;
  m_py << indent;
  m_py << tab << "try:" << endl;
  m_py << indent;
  m_py << tab << "self.receive_identity()" << endl;
  m_py << tab << "header = hermes.RequestHeader.recv(self.socket)" << endl;
  for (const auto& procedure_ : a_interface.procedures())
  {
    header_id++;
    server_method(procedure_, header_id);
  }
  m_py << tab << "else:" << endl;
  m_py << indent;
  m_py << tab << "self.send_identity()" << endl;
  m_py << tab << "raise hermes.HermesError('" << error_message << "')" << endl;
  m_py << unindent;
  m_py << unindent;
  m_py << tab << "except:" << endl;
  m_py << indent;
  m_py << tab << "self.send_identity()" << endl;
  m_py << tab << "hermes.ReplyHeader.create(0x00, False).send(self.socket)" << endl;
  m_py << unindent;
  m_py << unindent;
  m_py << endl;

}

void
generator::client_method(const state::procedure& a_procedure, int a_id)
{
  using std::endl;
  using std::ostream;
  typedef const state::field& field_t;

  auto argument = [](ostream& out, field_t f){ out << f.name(); };
  auto pack = [&](field_t p){ translate(p.type())->pack(m_py, p.name()); };

  auto name = a_procedure.name();
  auto result = a_procedure.result();
  auto params = a_procedure.parameters();
  auto errors = a_procedure.exceptions();
  auto has_args = !params.empty();
  auto is_void = result->is_void();

  auto id = to_hex(a_id);
  auto more = has_args ? "True" : "False";
  auto req_header = "hermes.RequestHeader.create(" + id + ", " + more + ").send(self.socket)";
  auto rep_header = "hermes.ReplyHeader.recv(self.socket)";
  auto error_message = "Unexpected result from \"" + name + "()\"";

  m_py << tab << "def " << name << "(self";
  if (has_args)
  {
    m_py << join(params.begin(), params.end(), argument).left(", ");
  }
  m_py << "):" << endl;
  m_py << indent;

  m_py << tab << req_header << endl;
  if (has_args)
  {
    m_py << tab << "buffer = bytearray()" << endl;
    std::for_each(params.begin(), params.end(), pack);
    m_py << tab << "self.socket.send(bytes(buffer))" << endl;
  }
  m_py << tab << "header = " << rep_header << endl;
  m_py << tab << "if header.number() == 0x01:" << endl;
  m_py << indent;
  if (is_void)
  {
    m_py << tab << "pass" << endl;
  }
  else
  {
    m_py << tab << "data = self.socket.recv()" << endl;
    m_py << tab << "position = 0" << endl;
    translate(result)->unpack(m_py, "result");
    m_py << tab << "return result" << endl;
  }
  m_py << unindent;

  auto eid = 1;
  for (auto err : errors)
  {
    eid++;
    m_py << tab << "elif header.number() == " << to_hex(eid) << ":" << endl;
    m_py << indent;
    m_py << tab << "data = self.socket.recv()" << endl;
    m_py << tab << "position = 0" << endl;
    translate(err)->unpack(m_py, "result");
    m_py << tab << "raise result" << endl;
    m_py << unindent;
  }

  m_py << tab << "else:" << endl;
  m_py << indent;
  m_py << tab << "raise hermes.HermesError('" << error_message << "')" << endl;
  m_py << unindent;

  m_py << unindent;
  m_py << endl;
}

void
generator::server_method(const state::procedure& a_procedure, int a_id)
{
  using std::endl;
  using std::ostream;
  typedef const state::field& field_t;

  auto argument = [](ostream& out, field_t f){ out << f.name(); };
  auto unpack = [&](field_t p){ translate(p.type())->unpack(m_py, p.name()); };

  auto name = a_procedure.name();
  auto result = a_procedure.result();
  auto params = a_procedure.parameters();
  auto errors = a_procedure.exceptions();
  auto has_args = !params.empty();
  auto has_errs = !errors.empty();
  auto is_void = result->is_void();

  auto id = to_hex(a_id);
  auto if_elif = (a_id == 1) ? "if" : "elif";
  auto reply_id = 1;

  m_py << tab << if_elif << " header.number() == " << id << ":" << endl;
  m_py << indent;
  if (has_args)
  {
    m_py << tab << "data = self.socket.recv()" << endl;
    m_py << tab << "position = 0" << endl;
    std::for_each(params.begin(), params.end(), unpack);
  }

  if (has_errs)
  {
    m_py << tab << "try:" << endl;
    m_py << indent;
  }

  m_py << tab;
  if (!is_void)
  {
    m_py << "result = ";
  }
  m_py << "self." << name;
  m_py << join(params.begin(), params.end(), argument).left("(").right(")");
  m_py << endl;

  if (!is_void)
  {
    m_py << tab << "self.send_identity()" << endl;
    m_py << tab << "hermes.ReplyHeader.create(0x01, True).send(self.socket)" << endl;
    m_py << tab << "buffer = bytearray()" << endl;
    translate(result)->pack(m_py, "result");
    m_py << tab << "self.socket.send(bytes(buffer))" << endl;
  }
  else
  {
    m_py << tab << "self.send_identity()" << endl;
    m_py << tab << "hermes.ReplyHeader.create(0x01, False).send(self.socket)" << endl;
  }

  if (has_errs)
  {
    m_py << unindent;
  }

  for (auto err : errors)
  {
    reply_id++;
    pointer type = translate(err);

    m_py << tab << "except " << type->name() << " as err:" << endl;
    m_py << indent;
    m_py << tab << "self.send_identity()" << endl;
    m_py << tab << "hermes.ReplyHeader.create(" << to_hex(reply_id) << ", True).send(self.socket)" << endl;
    m_py << tab << "buffer = bytearray()" << endl;
    type->pack(m_py, "err");
    m_py << tab << "self.socket.send(bytes(buffer))" << endl;
    m_py << unindent;
  }

  m_py << unindent;
}

} // python namespace
} // generator namespace
} // compiler namespace
} // hermes namespace
