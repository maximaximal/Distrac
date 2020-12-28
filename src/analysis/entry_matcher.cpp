#include <distrac/analysis/definition.hpp>
#include <distrac/analysis/entry_matcher.hpp>
#include <distrac/analysis/event.hpp>
#include <distrac/analysis/event_definition.hpp>
#include <distrac/analysis/event_iterator.hpp>
#include <distrac/analysis/node.hpp>
#include <distrac/analysis/property.hpp>
#include <distrac/analysis/property_definition.hpp>

#include <boost/functional/hash.hpp>
#include <iostream>

// Taken from https://stackoverflow.com/a/10405129
template<typename Container>
struct container_hash {
  std::size_t operator()(Container const& c) const {
    return boost::hash_range(c.begin(), c.end());
  }
};

namespace distrac {
entrymatcher::entrymatcher(const definition& def,
                           const event_definition& ev_def,
                           const nodevector& nodes)
  : _def(def)
  , _ev_def(ev_def)
  , _nodes(nodes) {}
entrymatcher::~entrymatcher() {}

entrymatcher::offsetvector
entrymatcher::run() {
  offsetvector offsets(_nodes.size());

  event_iterator it;

  assert(_ev_def.has_causal_dependency());
  auto depid = _ev_def.causal_dependency_event_id();
  assert(depid < _def.definitions().size());

  for(const auto& node : _nodes) {
    it.add_event(node.get_event(depid));
    it.add_event(node.get_event(_ev_def.id()));
  }

  // Iterate over all events and match the dependency to this event using an
  // unordered_map.
  std::unordered_map<memoryvector, int64_t, container_hash<memoryvector>>
    memory;

  memoryvector key;

  for(; it != event_iterator(); ++it) {
    const auto& ev = *it;
    criticalMemoryToVector(key, ev);

    auto memoryIt = memory.find(key);

    if(memoryIt != memory.end()) {
      if(ev.id() == _ev_def.id()) {
        // Nothing else to do, as this event is already after its dependency.
      } else {
        // The dependency is AFTER the event that would depend on it! Offset
        // must be applied to the node the event was found in.
        auto& off = offsets[ev.node().tracefile_location_index()];
        off = std::min(off, memoryIt->second - ev.timestamp_with_offset());
      }

      memory.erase(memoryIt);
    } else {
      memory.insert({ key, ev.timestamp_with_offset() });
    }
  }

  return offsets;
}

void
entrymatcher::criticalMemoryToVector(std::vector<uint8_t>& out,
                                     const event& source) {
  out.clear();
  for(const auto& prop : _ev_def.definitions()) {
    if(prop.has_causal_dependency()) {
      const auto& p = [this, source, prop]() {
        if(source.id() == _ev_def.id()) {
          return source.property(prop.id());
        } else {
          return source.property(prop.causal_dependency_property_id());
        }
      }();

      auto begin = p.memory();
      auto end = p.memory() + p.size();

      std::copy(begin, end, std::back_inserter(out));
    }
  }
}

}
