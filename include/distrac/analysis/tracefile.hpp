#pragma once

#include <map>
#include <string>
#include <vector>

#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/type_index.hpp>

#include "node.hpp"

namespace distrac {
#include "distrac/distrac.h"
#include "distrac/headers.h"

class event_definition;

class tracefile {
  public:
  using event_definition_vector = std::vector<event_definition>;

  tracefile(const std::string& path);
  ~tracefile();

  void print_summary();

  const event_definition_vector& event_definitions() const {
    return _event_definitions;
  }

  protected:
  void scan();

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
  const T& read_struct(size_t& pos) {
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

  event_definition_vector _event_definitions;

  boost::iostreams::mapped_file _sink;

  distrac_file_header* _header;
};
}
