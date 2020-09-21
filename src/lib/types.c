#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <distrac/types.h>

const char*
distrac_type_to_str(enum distrac_type type) {
  switch(type) {
    case DISTRAC_TYPE_UINT8:
      return "uint8";
    case DISTRAC_TYPE_INT8:
      return "int8";
    case DISTRAC_TYPE_UINT16:
      return "uint16";
    case DISTRAC_TYPE_INT16:
      return "int16";
    case DISTRAC_TYPE_UINT32:
      return "uint32";
    case DISTRAC_TYPE_INT32:
      return "int32";
    case DISTRAC_TYPE_UINT64:
      return "uint64";
    case DISTRAC_TYPE_INT64:
      return "int64";
    case DISTRAC_TYPE_IPV4:
      return "ipv4";
    case DISTRAC_TYPE_IPV6:
      return "ipv6";
    case DISTRAC_TYPE_DOUBLE:
      return "double";
    case DISTRAC_TYPE_FLOAT:
      return "float";
    case DISTRAC_TYPE_BOOL:
      return "bool";
    case DISTRAC_TYPE__COUNT:
      assert(false);
  }
}

size_t
distrac_type_sizeof(enum distrac_type type) {
  switch(type) {
    case DISTRAC_TYPE_UINT8:
      return sizeof(uint8_t);
    case DISTRAC_TYPE_INT8:
      return sizeof(int8_t);
    case DISTRAC_TYPE_UINT16:
      return sizeof(uint16_t);
    case DISTRAC_TYPE_INT16:
      return sizeof(int16_t);
    case DISTRAC_TYPE_UINT32:
      return sizeof(uint32_t);
    case DISTRAC_TYPE_INT32:
      return sizeof(int32_t);
    case DISTRAC_TYPE_UINT64:
      return sizeof(uint64_t);
    case DISTRAC_TYPE_INT64:
      return sizeof(int64_t);
    case DISTRAC_TYPE_IPV4:
      return sizeof(uint8_t) * 4;
    case DISTRAC_TYPE_IPV6:
      return sizeof(uint8_t) * 16;
    case DISTRAC_TYPE_DOUBLE:
      return sizeof(double);
    case DISTRAC_TYPE_FLOAT:
      return sizeof(float);
    case DISTRAC_TYPE_BOOL:
      return sizeof(uint8_t);
    case DISTRAC_TYPE__COUNT:
      assert(false);
  }
}

size_t
distrac_type_required_padding(enum distrac_type type) {
  switch(type) {
    case DISTRAC_TYPE_UINT8:
    case DISTRAC_TYPE_INT8:
    case DISTRAC_TYPE_BOOL:
    case DISTRAC_TYPE_IPV4:
    case DISTRAC_TYPE_IPV6:
      return 0;
    case DISTRAC_TYPE_UINT16:
    case DISTRAC_TYPE_INT16:
      return 1;
    case DISTRAC_TYPE_UINT32:
    case DISTRAC_TYPE_INT32:
    case DISTRAC_TYPE_FLOAT:
      return 2;
    case DISTRAC_TYPE_UINT64:
    case DISTRAC_TYPE_INT64:
    case DISTRAC_TYPE_DOUBLE:
      return 3;
    case DISTRAC_TYPE__COUNT:
      assert(false);
  }
}
