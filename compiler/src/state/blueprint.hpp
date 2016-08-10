#ifndef HERMES_COMPILER_STATE_BLUEPRINT_HPP
#define HERMES_COMPILER_STATE_BLUEPRINT_HPP

#include <cassert>
#include <memory>
#include <set>
#include <stack>
#include <vector>
#include "state/alias.hpp"
#include "state/constant.hpp"
#include "state/container.hpp"
#include "state/datatype.hpp"
#include "state/enumeration.hpp"
#include "state/exception.hpp"
#include "state/interface.hpp"
#include "state/predicates.hpp"
#include "state/space.hpp"
#include "state/structure.hpp"

namespace hermes {
namespace compiler {
namespace state {

template <typename Rule> class action;

class blueprint
{
  template <typename Rule> friend class action;
public:
  typedef std::shared_ptr<datatype> pointer;

  const std::set<space>& spaces() const;
  const std::set<constant>& constants() const;
  const std::set<interface>& interfaces() const;
  const std::set<std::shared_ptr<enumeration>>& enumerations() const;
  const std::set<std::shared_ptr<structure>>& structures() const;
  const std::set<std::shared_ptr<exception>>& exceptions() const;
  const std::vector<std::shared_ptr<alias>>& aliases() const;

  pointer find(const std::string& a_name) const;
  std::string get_space(const std::string& a_name) const;
protected:
  std::string token();
  void token(const std::string& a_identifier);

  pointer datatype();
  void datatype(pointer a_datatype);

  void make_alias();
  void make_constant();
  void make_enumeration();
  void make_enumeration_item();
  void make_exception();
  void make_interface();
  void make_map();
  void make_procedure();
  void make_set();
  void make_space();
  void make_structure();
  void make_vector();

  void store_enumeration();
  void store_interface();
private:
  std::stack<std::string> m_tokens;
  std::stack<pointer> m_datatypes;
  std::shared_ptr<enumeration> m_working_enumeration;
  std::shared_ptr<interface> m_working_interface;

  std::set<space> m_spaces;
  std::set<constant> m_constants;
  std::set<interface> m_interfaces;
  std::set<std::shared_ptr<enumeration>> m_enumerations;
  std::set<std::shared_ptr<structure>> m_structures;
  std::set<std::shared_ptr<exception>> m_exceptions;
  std::vector<std::shared_ptr<alias>> m_aliases;
}; // blueprint class

} // state namespace
} // compiler namespace
} // hermes namespace

#endif // HERMES_COMPILER_STATE_BLUEPRINT_HPP