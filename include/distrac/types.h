#ifndef DISTRAC_TYPES_INCLUDED
#define DISTRAC_TYPES_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

enum distrac_type {
  DISTRAC_TYPE_UINT8,
  DISTRAC_TYPE_INT8,
  DISTRAC_TYPE_UINT16,
  DISTRAC_TYPE_INT16,
  DISTRAC_TYPE_UINT32,
  DISTRAC_TYPE_INT32,
  DISTRAC_TYPE_UINT64,
  DISTRAC_TYPE_INT64,
  DISTRAC_TYPE_IPV4,
  DISTRAC_TYPE_IPV6,
  DISTRAC_TYPE_DOUBLE,
  DISTRAC_TYPE_FLOAT,
  DISTRAC_TYPE_BOOL,
  DISTRAC_TYPE__COUNT
};

typedef struct distrac_ipv4 {
  uint8_t segments[4];
} distrac_ipv4;

typedef struct distrac_ipv6 {
  uint8_t segments[16];
} distrac_ipv6;

typedef int64_t distrac_id;

const char*
distrac_type_to_str(enum distrac_type type);

size_t
distrac_type_sizeof(enum distrac_type type);

// Either 0, 1, 2 or 3. Specifies the shift count.
size_t
distrac_type_required_padding(enum distrac_type type);

#ifdef __cplusplus
}

template<typename Functor>
void
distrac_memory_to_type(const uint8_t* mem,
                       enum distrac_type type,
                       Functor func) {
  switch(type) {
    case DISTRAC_TYPE_UINT8:
      return func(*reinterpret_cast<const uint8_t*>(mem));
    case DISTRAC_TYPE_INT8:
      return func(*reinterpret_cast<const int8_t*>(mem));
    case DISTRAC_TYPE_UINT16:
      return func(*reinterpret_cast<const uint16_t*>(mem));
    case DISTRAC_TYPE_INT16:
      return func(*reinterpret_cast<const int16_t*>(mem));
    case DISTRAC_TYPE_UINT32:
      return func(*reinterpret_cast<const uint32_t*>(mem));
    case DISTRAC_TYPE_INT32:
      return func(*reinterpret_cast<const int32_t*>(mem));
    case DISTRAC_TYPE_UINT64:
      return func(*reinterpret_cast<const uint64_t*>(mem));
    case DISTRAC_TYPE_INT64:
      return func(*reinterpret_cast<const int64_t*>(mem));
    case DISTRAC_TYPE_IPV4:
      return func(*reinterpret_cast<const distrac_ipv4*>(mem));
    case DISTRAC_TYPE_IPV6:
      return func(*reinterpret_cast<const distrac_ipv6*>(mem));
    case DISTRAC_TYPE_DOUBLE:
      return func(*reinterpret_cast<const double*>(mem));
    case DISTRAC_TYPE_FLOAT:
      return func(*reinterpret_cast<const float*>(mem));
    case DISTRAC_TYPE_BOOL:
      return func(*reinterpret_cast<const uint8_t*>(mem));

    case DISTRAC_TYPE__COUNT:
      assert(false);
  }
}

#include <ostream>
inline std::ostream&
operator<<(std::ostream& o, const distrac_type t) {
  return o << distrac_type_to_str(t);
}

#if __cplusplus >= 201703L
#include <variant>

typedef std::variant<uint8_t,
                     int8_t,
                     uint16_t,
                     int16_t,
                     uint32_t,
                     int32_t,
                     uint64_t,
                     int64_t,
                     distrac_ipv4,
                     distrac_ipv6,
                     double,
                     float,
                     bool>
  distrac_type_variant;

inline void
distrac_memory_to_variant_assign(const uint8_t* mem,
                                 enum distrac_type type,
                                 distrac_type_variant& variant) {
  distrac_memory_to_type(mem, type, [&variant](auto&& T) { variant = T; });
}

inline distrac_type_variant
distrac_memory_to_variant(const uint8_t* mem, enum distrac_type type) {
  distrac_type_variant variant;
  distrac_memory_to_variant_assign(mem, type, variant);
  return variant;
}
#endif

#endif

#endif
