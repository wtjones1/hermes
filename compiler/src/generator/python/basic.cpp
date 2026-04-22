#include "generator/python/basic.hpp"

namespace hermes {
namespace compiler {
namespace generator {
namespace python {

basic::basic(const std::string& a_name, const std::string& a_type)
  : datatype(a_name)
  , m_type(a_type)
{
  // empty
}

void
basic::pack(std::ostream& a_out, const std::string& a_variable) const
{
  std::string format;
  if (m_type == "bool") format = ">I";
  else if (m_type == "int") format = ">i";
  else if (m_type == "uint") format = ">I";
  else if (m_type == "hyper") format = ">q";
  else if (m_type == "uhyper") format = ">Q";
  else if (m_type == "float") format = ">f";
  else if (m_type == "double") format = ">d";
  
  a_out << tab << "buffer.extend(struct.pack('" << format << "', " << a_variable << "))" << std::endl;
}

void
basic::unpack(std::ostream& a_out, const std::string& a_variable) const
{
  std::string format;
  int size;
  
  if (m_type == "bool") { format = ">I"; size = 4; }
  else if (m_type == "int") { format = ">i"; size = 4; }
  else if (m_type == "uint") { format = ">I"; size = 4; }
  else if (m_type == "hyper") { format = ">q"; size = 8; }
  else if (m_type == "uhyper") { format = ">Q"; size = 8; }
  else if (m_type == "float") { format = ">f"; size = 4; }
  else if (m_type == "double") { format = ">d"; size = 8; }
  
  a_out << tab << a_variable << " = struct.unpack_from('" << format << "', data, position)[0]" << std::endl;
  a_out << tab << "position += " << size << std::endl;
}

bool_t::bool_t()
  : basic("bool", "bool")
{
  // empty
}

std::string
bool_t::default_value() const
{
  return "False";
}

char_t::char_t()
  : basic("str", "fstring")
{
  // empty
}

std::string
char_t::default_value() const
{
  return "' '";
}

void
char_t::pack(std::ostream& a_out, const std::string& a_variable) const
{
  a_out << tab << "if sys.version_info[0] < 3:" << std::endl;
  a_out << indent;
  a_out << tab << "c = ord(" << a_variable << ") if isinstance(" << a_variable << ", str) else ord(chr(" << a_variable << "))" << std::endl;
  a_out << unindent;
  a_out << tab << "else:" << std::endl;
  a_out << indent;
  a_out << tab << "c = ord(" << a_variable << ") if isinstance(" << a_variable << ", str) else " << a_variable << std::endl;
  a_out << unindent;
  a_out << tab << "buffer.extend(struct.pack('>I', c))" << std::endl;
}

void
char_t::unpack(std::ostream& a_out, const std::string& a_variable) const
{
  a_out << tab << "c = struct.unpack_from('>I', data, position)[0]" << std::endl;
  a_out << tab << "position += 4" << std::endl;
  a_out << tab << a_variable << " = chr(c)" << std::endl;
}

int8::int8()
  : basic("int", "int")
{
  // empty
}

std::string
int8::default_value() const
{
  return "0";
}

int16::int16()
  : basic("int", "int")
{
  // empty
}

std::string
int16::default_value() const
{
  return "0";
}

int32::int32()
  : basic("int", "int")
{
  // empty
}

std::string
int32::default_value() const
{
  return "0";
}

int64::int64()
  : basic("long", "hyper")
{
  // empty
}

std::string
int64::default_value() const
{
  return "0";
}

uint8::uint8()
  : basic("int", "uint")
{
  // empty
}

std::string
uint8::default_value() const
{
  return "0";
}

uint16::uint16()
  : basic("int", "uint")
{
  // empty
}

std::string
uint16::default_value() const
{
  return "0";
}

uint32::uint32()
  : basic("long", "uint")
{
  // empty
}

std::string
uint32::default_value() const
{
  return "0";
}

uint64::uint64()
  : basic("long", "uhyper")
{
  // empty
}

std::string
uint64::default_value() const
{
  return "0";
}

real32::real32()
  : basic("float", "float")
{
  // empty
}

std::string
real32::default_value() const
{
  return "0.0";
}

real64::real64()
  : basic("float", "double")
{
  // empty
}

std::string
real64::default_value() const
{
  return "0.0";
}

string::string()
  : basic("str", "string")
{
  // empty
}

std::string
string::default_value() const
{
  return "''";
}

void
string::pack(std::ostream& a_out, const std::string& a_variable) const
{
  a_out << tab << "hermes.xdr_pack_string(buffer, " << a_variable << ")" << std::endl;
}

void
string::unpack(std::ostream& a_out, const std::string& a_variable) const
{
  a_out << tab << a_variable << ", position = hermes.xdr_unpack_string(data, position)" << std::endl;
}

} // python namespace
} // generator namespace
} // compiler namespace
} // hermes namespace
