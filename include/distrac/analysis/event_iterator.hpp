#pragma once

#include <algorithm>
#include <cassert>
#include <distrac/analysis/event.hpp>
#include <distrac/types.h>
#include <forward_list>
#include <functional>

namespace distrac {
class event_iterator {
  public:
  typedef event_iterator self_type;
  typedef event value_type;
  typedef const event& reference;
  typedef const event* pointer;
  typedef std::forward_iterator_tag iterator_category;
  typedef size_t difference_type;
  typedef std::forward_list<event> event_list;

  event_iterator(bool respect_offset = true)
    : _respect_offset(respect_offset) {}
  template<typename Iter>
  event_iterator(Iter Begin, Iter End) {
    add_events(Begin, End);
  }
  event_iterator(const self_type& self) = default;

  template<typename Iter>
  void add_events(Iter Begin, Iter End) {
    std::copy(Begin, End, std::front_inserter(_events));
    _event_count += std::distance(Begin, End);
    recompute_current_event();
  }
  void add_event(const event& e) {
    _events.push_front(e);
    ++_event_count;
    recompute_current_event();
  }

  const event_list& events() const { return _events; }

  size_t event_count() const { return _event_count; }

  void recompute_current_event() {
    if(_events.empty()) {
      _current_event = nullptr;
      return;
    }

    const auto& min =
      std::min_element(_events.begin(),
                       _events.end(),
                       std::bind(&event_iterator::compare_events_by_time,
                                 this,
                                 std::placeholders::_1,
                                 std::placeholders::_2));
    _current_event = &(*min);

    if(_current_event && !_current_event->valid()) {
      _current_event = nullptr;
    }
  }

  bool compare_events_by_time(const event& lhs, const event& rhs) {
    if(lhs.valid() && !rhs.valid())
      return true;
    if(!lhs.valid() && rhs.valid())
      return false;
    if(!lhs.valid() && !rhs.valid())
      return false;

    if(_respect_offset) {
      return lhs.timestamp_with_offset() < rhs.timestamp_with_offset();
    } else {
      return lhs.timestamp() < rhs.timestamp();
    }
  }

  // PREFIX
  self_type operator++() {
    if(_current_event) {
      ++(*_current_event);

      if(!_current_event->valid()) {
        _events.remove(*_current_event);
        _current_event = nullptr;
      }
    }

    recompute_current_event();

    return *this;
  }

  // POSTFIX
  self_type operator++(int junk) {
    (void)junk;
    self_type self = *this;
    ++(*this);
    return self;
  }

  reference operator*() {
    assert(_current_event);
    return *_current_event;
  }
  pointer operator->() { return &(**this); }

  event_iterator operator+(size_t i) {
    self_type it = *this;
    for(size_t j = 0; j < i; ++j) {
      ++it;
    }
    return it;
  }

  bool operator==(const self_type& rhs) const {
    if(_current_event == rhs._current_event)
      return true;
    if(_current_event == nullptr || rhs._current_event == nullptr)
      return false;

    return *_current_event == *(rhs._current_event) &&
           event_count() == rhs.event_count() &&
           _current_event->number() == rhs._current_event->number() &&
           _current_event->node_id() == rhs._current_event->node_id();
  }
  bool operator!=(const self_type& rhs) const { return !(*this == rhs); }

  private:
  event_list _events;
  size_t _event_count = 0;
  event* _current_event = nullptr;
  bool _respect_offset = true;
};
}
