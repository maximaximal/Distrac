#include "distrac/types.h"
#include <distrac/analysis/event_definition.hpp>
#include <distrac/analysis/property_definition.hpp>

namespace distrac {
size_t
property_definition::offset(const event_definition& ev) {
  size_t offset = 0;
  for(size_t prop_id = 0; prop_id < ev.property_count(); ++prop_id) {
    offset += ev.property_size(prop_id);
  }
  return offset;
}

bool
property_definition::is_correctly_aligned(const event_definition& ev) const {
  size_t offset = property_definition::offset(ev);
  size_t padding = distrac_type_required_padding(_type);
  for(size_t i = padding; i > 0; --i) {
    if(offset & (0b00000001U << (i - 1) != 0)) {
      return false;
    }
  }
  return true;
}
}
