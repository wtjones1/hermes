#ifndef HERMES_COMPILER_GENERATOR_PYTHON_DATATYPE_HPP
#define HERMES_COMPILER_GENERATOR_PYTHON_DATATYPE_HPP

#include <memory>
#include <iostream>
#include <string>
#include "generator/iomanip.hpp"

namespace hermes {
namespace compiler {
namespace generator {
namespace python {

class datatype
{
public:
  datatype() = delete;
  datatype(datatype&&) = delete;
  datatype(const datatype &) = delete;
  datatype& operator=(datatype&&) = delete;
  datatype& operator=(const datatype&) = delete;

  virtual ~datatype();

  virtual std::string name() const;
  virtual std::string default_value() const = 0;
  virtual void pack(std::ostream& a_out, const std::string& a_variable) const;
  virtual void unpack(std::ostream& a_out, const std::string& a_variable) const;
protected:
  datatype(const std::string& a_name);
private:
  std::string m_name;
};

} // python namespace
} // generator namespace
} // compiler namespace
} // hermes namespace

#endif // HERMES_COMPILER_GENERATOR_PYTHON_DATATYPE_HPP
