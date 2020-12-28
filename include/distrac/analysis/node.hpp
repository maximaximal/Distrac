#pragma once

#include <string>
#include <vector>

#include <distrac/headers.h>
#include <distrac/types.h>

namespace distrac {
class tracefile;
class event_iterator;
class event;

class node {
  public:
  node(const distrac_node_header& header,
       const tracefile& trace,
       size_t tracefile_location_index);
  ~node();

  const distrac_id& id() const { return _id; }
  size_t tracefile_location_index() const { return _tracefile_location_index; }
  const std::string& name() const { return _name; }
  const std::string& hostname() const { return _hostname; }
  const std::string& program() const { return _program; }
  const distrac_version& version() const { return _distrac_verion; }
  int64_t offset_ns() const { return _offset_ns; }

  size_t following_size() const { return _following_size; }

  size_t event_count(uint8_t ev) const { return _event_counts.at(ev); }

  const uint8_t* get_event_memory(uint8_t event, uint64_t number) const;

  event_iterator begin() const;
  event_iterator end() const;

  event get_event(uint8_t id, uint64_t number = 0) const;

  private:
  const distrac_node_header& _header;
  distrac_id _id;
  size_t _tracefile_location_index;
  std::string _name;
  std::string _hostname;
  std::string _program;
  distrac_version _distrac_verion;
  int64_t _offset_ns;

  size_t _following_size;

  std::vector<uint64_t> _event_counts;
  std::vector<const uint8_t*> _event_memory_locations;

  size_t compute_following_size();

  const tracefile& _trace;
};
}
