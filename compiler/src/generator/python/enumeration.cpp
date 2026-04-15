#include "generator/python/enumeration.hpp"

namespace hermes {
namespace compiler {
namespace generator {
namespace python {

enumeration::enumeration()
  : datatype("int")
{
  // empty
}

std::string
enumeration::default_value() const
{
  return "0";
}

void
enumeration::pack(std::ostream& a_out, const std::string& a_variable) const
{
  a_out << tab << "buffer.extend(struct.pack('>i', " << a_variable << "))" << std::endl;
}

void
enumeration::unpack(std::ostream& a_out, const std::string& a_variable) const
{
  a_out << tab << a_variable << " = struct.unpack_from('>i', data, position)[0]" << std::endl;
  a_out << tab << "position += 4" << std::endl;
}

} // python namespace
} // generator namespace
} // compiler namespace
} // hermes namespace
