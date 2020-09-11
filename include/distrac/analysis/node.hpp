#pragma once

#include <string>
#include <vector>

#include <distrac/headers.h>
#include <distrac/types.h>

namespace distrac {
class tracefile;

class node {
  public:
  node(const distrac_node_header& header, const tracefile& trace);
  ~node();

  distrac_id id() const { return _id; }
  const std::string& name() const { return _name; }
  const std::string& program() const { return _program; }
  const distrac_version& version() const { return _distrac_verion; }
  int64_t offset_ns() const { return _offset_ns; }

  size_t following_size() const { return _following_size; }

  size_t event_count(uint8_t ev) const { return _event_counts.at(ev); }

  private:
  const distrac_node_header& _header;
  distrac_id _id;
  std::string _name;
  std::string _program;
  distrac_version _distrac_verion;
  int64_t _offset_ns;

  size_t _following_size;

  std::vector<uint64_t> _event_counts;
  std::vector<const uint8_t*> _event_offsets;

  size_t compute_following_size();

  const tracefile& _trace;
};
}
