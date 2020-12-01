#ifndef DISTRAC_TYPES_INCLUDED
#define DISTRAC_TYPES_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#define DISTRAC_PARAC_PATH_MAX_LENGTH 58
#define DISTRAC_PARAC_PATH_EXPLICITLY_UNKNOWN 0b00111110u
#define DISTRAC_PARAC_PATH_PARSER 0b00111101u
#define DISTRAC_PARAC_PATH_BITS 58u
#define DISTRAC_PARAC_PATH_LENGTH_BITS 6u

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
  DISTRAC_TYPE_PARAC_PATH,
  DISTRAC_TYPE__COUNT
};

typedef struct distrac_ipv4 {
  uint8_t segments[4];
} distrac_ipv4;

typedef struct distrac_ipv6 {
  uint16_t segments[8];
} distrac_ipv6;

typedef struct __attribute__((__packed__)) distrac_parac_path {
  union __attribute__((__packed__)) {
    struct __attribute__((__packed__)) {
      uint8_t length : 6;
      uint64_t path : 58;
    };
    uint64_t rep : 64;
  };
} distrac_parac_path;

typedef int64_t distrac_id;

const char*
distrac_type_to_str(enum distrac_type type);

const char*
distrac_type_to_cname(enum distrac_type type);

size_t
distrac_type_sizeof(enum distrac_type type);

void
distrac_parac_path_to_str(distrac_parac_path p, char* out_str);

// Either 0, 1, 2 or 3. Specifies the shift count.
size_t
distrac_type_required_padding(enum distrac_type type);

#ifdef __cplusplus
}
#include <iomanip>
#include <ostream>

inline std::ostream&
operator<<(std::ostream& o, const distrac_ipv4& ipv4) {
  return o << static_cast<int>(ipv4.segments[0]) << "."
           << static_cast<int>(ipv4.segments[0]) << "."
           << static_cast<int>(ipv4.segments[0]) << "."
           << static_cast<int>(ipv4.segments[0]);
}

inline std::ostream&
operator<<(std::ostream& o, const distrac_ipv6& ipv6) {
  std::ios ostream_state(nullptr);
  ostream_state.copyfmt(o);

  for(size_t i = 0; i < 16; ++i) {
    o << std::setfill('0') << std::setw(4) << std::hex << ipv6.segments[i];
  }

  o.copyfmt(ostream_state);
  return o;
}

inline std::ostream&
operator<<(std::ostream& o, const distrac_parac_path p) {
  char out_str[64];
  distrac_parac_path_to_str(p, out_str);
  return o << out_str;
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
    case DISTRAC_TYPE_PARAC_PATH:
      return func(*reinterpret_cast<const distrac_parac_path*>(mem));

    case DISTRAC_TYPE__COUNT:
      assert(false);
  }
}

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
                     bool,
                     distrac_parac_path>
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
