#pragma once

#include <cstdint>
#include <vector>

namespace distrac {
class definition;
class node;
class event;
class event_definition;

/* Compute the minimal required so that the given event is shifted to
 * after all its causal predecessors. */
class entrymatcher {
  public:
  using nodevector = std::vector<node>;
  using offsetvector = std::vector<int64_t>;
  using memoryvector = std::vector<uint8_t>;

  entrymatcher(const definition& def,
               const event_definition& ev_def,
               const nodevector& nodes);
  ~entrymatcher();

  offsetvector run();

  private:
  const definition& _def;
  const event_definition& _ev_def;
  const nodevector& _nodes;

  void criticalMemoryToVector(std::vector<uint8_t>& out, const event& source);
};
}
