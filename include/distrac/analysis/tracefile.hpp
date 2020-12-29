#pragma once

#include <chrono>
#include <map>
#include <set>
#include <string>
#include <vector>

#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/type_index.hpp>

#include "node.hpp"

#include "distrac/distrac.h"
#include "distrac/headers.h"

#include "distrac/analysis/definition.hpp"

namespace distrac {

class event_definition;
class event_iterator;

class tracefile {
  public:
  using event_definition_vector = definition::event_definition_vector;

  tracefile(const std::string& path);
  ~tracefile();

  void print_summary();

  const event_definition_vector& event_definitions() const;
  ssize_t get_event_id(const std::string& name) const;

  std::chrono::time_point<std::chrono::system_clock> trace_time_point() {
    return std::chrono::system_clock::time_point{ std::chrono::seconds{
      _header->seconds_since_epoch_on_start } };
  }
  std::time_t trace_time() {
    return std::chrono::system_clock::to_time_t(trace_time_point());
  }

  event_iterator begin() const;
  event_iterator end() const;

  using event_filter_func = std::function<bool(const event&)>;
  using event_filter_set = std::set<uint8_t>;

  struct filtered_tracefile {
    event_filter_func func;
    const tracefile& trace;

    event_iterator begin() const;
    event_iterator end() const;
  };

  size_t event_count(uint8_t ev) const;

  filtered_tracefile filtered(event_filter_func func) const;
  filtered_tracefile filtered(const event_filter_set& event_ids) const;

  protected:
  void scan();
  void calculate_offsets();

  template<typename T>
  T* try_read_struct(size_t& pos) {
    if(is_size_left(pos, sizeof(T))) {
      T* s = reinterpret_cast<T*>(_sink.data() + pos);
      pos += sizeof(T);
      return s;
    } else {
      return nullptr;
    }
  }

  template<typename T>
  T& read_struct(size_t& pos) {
    assert_size_left(
      pos, sizeof(T), boost::typeindex::type_id<T>().pretty_name().c_str());
    T& s = *reinterpret_cast<T*>(&_sink.data()[pos]);
    pos += sizeof(T);
    return s;
  }

  template<typename T>
  const T& read_struct(size_t& pos) const {
    assert_size_left(
      pos, sizeof(T), boost::typeindex::type_id<T>().pretty_name().c_str());
    const T& s = *reinterpret_cast<T*>(&_sink.data()[pos]);
    pos += sizeof(T);
    return s;
  }

  void read_until_aligned(size_t& pos) { pos += 8 - (pos & 0b00000111); }

  inline bool is_size_left(size_t pos, size_t size) {
    return _byte_size - pos > size;
  }
  void assert_size_left(size_t pos, size_t size, const char* structname);

  private:
  std::string _path;
  std::size_t _byte_size;

  std::vector<node> _nodes;
  std::map<int64_t, node&> _id_to_node;

  definition _definition;

  boost::iostreams::mapped_file _sink;

  distrac_file_header* _header = nullptr;
};
}
