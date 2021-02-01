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

  tracefile(const std::string& path, bool force_causal_sync = false);
  ~tracefile();

  void print_summary();

  const event_definition_vector& event_definitions() const;
  ssize_t get_event_id(const std::string& name) const;

  std::time_t trace_time() const {
    return std::chrono::system_clock::to_time_t(trace_begin_time());
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

  using time_point = std::chrono::time_point<std::chrono::system_clock>;

  const time_point& trace_begin_time() const { return _begin_time; }
  const time_point& trace_end_time() const { return _end_time; }
  int64_t trace_duration_ms() const {
    return std::chrono::duration_cast<std::chrono::milliseconds>(_end_time -
                                                                 _begin_time)
      .count();
  }
  int64_t trace_internal_begin_time_ns() const { return _begin_time_ns; };
  int64_t trace_internal_end_time_ns() const { return _end_time_ns; };

  size_t node_count() const { return _nodes.size(); }

  protected:
  void scan();
  void calculate_offsets();
  void calculate_beginAndEndTime();

  bool requiresOffsetRecalculation();

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

  time_point _begin_time;
  time_point _end_time;
  int64_t _begin_time_ns;
  int64_t _end_time_ns;
};
}
