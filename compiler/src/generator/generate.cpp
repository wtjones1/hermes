#include "generator/generate.hpp"

namespace hermes {
namespace compiler {
namespace generator {

using state::blueprint;

void generate(const std::string& a_language,
              const std::string& a_project,
              const std::string& a_directory,
              const blueprint& a_blueprint)
{
  if (a_language == "cpp")
  {
    cpp::generator gen;
    gen.open(a_project, a_directory);
    gen.write(a_blueprint);
    gen.close();
  }
  else if (a_language == "fortran")
  {
    fortran::generator gen;
    gen.open(a_project, a_directory);
    gen.write(a_blueprint);
    gen.close();
  }
  else if (a_language == "python")
  {
    python::generator gen;
    gen.open(a_project, a_directory);
    gen.write(a_blueprint);
    gen.close();
  }
}

} // generator namespace
} // compiler namespace
} // hermes namespace
