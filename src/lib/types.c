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
    case DISTRAC_TYPE_PARAC_PATH:
      return "parac_path";
    case DISTRAC_TYPE__COUNT:
      assert(false);
  }
}

const char*
distrac_type_to_cname(enum distrac_type type) {
  switch(type) {
    case DISTRAC_TYPE_UINT8:
      return "uint8_t";
    case DISTRAC_TYPE_INT8:
      return "int8_t";
    case DISTRAC_TYPE_UINT16:
      return "uint16_t";
    case DISTRAC_TYPE_INT16:
      return "int16_t";
    case DISTRAC_TYPE_UINT32:
      return "uint32_t";
    case DISTRAC_TYPE_INT32:
      return "int32_t";
    case DISTRAC_TYPE_UINT64:
      return "uint64_t";
    case DISTRAC_TYPE_INT64:
      return "int64_t";
    case DISTRAC_TYPE_IPV4:
      return "distrac_ipv4";
    case DISTRAC_TYPE_IPV6:
      return "distrac_ipv6";
    case DISTRAC_TYPE_DOUBLE:
      return "double";
    case DISTRAC_TYPE_FLOAT:
      return "float";
    case DISTRAC_TYPE_BOOL:
      return "bool";
    case DISTRAC_TYPE_PARAC_PATH:
      return "distrac_parac_path";
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
    case DISTRAC_TYPE_PARAC_PATH:
      return sizeof(distrac_parac_path);
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
      return 0;
    case DISTRAC_TYPE_UINT16:
    case DISTRAC_TYPE_INT16:
    case DISTRAC_TYPE_IPV6:
      return 1;
    case DISTRAC_TYPE_UINT32:
    case DISTRAC_TYPE_INT32:
    case DISTRAC_TYPE_FLOAT:
      return 2;
    case DISTRAC_TYPE_UINT64:
    case DISTRAC_TYPE_INT64:
    case DISTRAC_TYPE_DOUBLE:
    case DISTRAC_TYPE_PARAC_PATH:
      return 3;
    case DISTRAC_TYPE__COUNT:
      assert(false);
  }
}

static const uint64_t long1 = 1u;

static bool
distrac_parac_path_get_assignment(distrac_parac_path p, uint8_t d) {
  assert(d <= DISTRAC_PARAC_PATH_MAX_LENGTH);
  return p.rep & (long1 << ((sizeof(distrac_parac_path) * 8) - 1 - (d - 1)));
}

void
distrac_parac_path_to_str(distrac_parac_path p, char* out_str) {
  if(p.length == 0) {
    size_t i = 0;
    out_str[i++] = '(';
    out_str[i++] = 'r';
    out_str[i++] = 'o';
    out_str[i++] = 'o';
    out_str[i++] = 't';
    out_str[i++] = ')';
    out_str[i++] = '\0';
    return;
  } else if(p.length == DISTRAC_PARAC_PATH_EXPLICITLY_UNKNOWN) {
    size_t i = 0;
    out_str[i++] = '(';
    out_str[i++] = 'e';
    out_str[i++] = 'x';
    out_str[i++] = 'p';
    out_str[i++] = 'l';
    out_str[i++] = 'i';
    out_str[i++] = 'c';
    out_str[i++] = 'i';
    out_str[i++] = 't';
    out_str[i++] = 'l';
    out_str[i++] = 'y';
    out_str[i++] = ' ';
    out_str[i++] = 'u';
    out_str[i++] = 'n';
    out_str[i++] = 'k';
    out_str[i++] = 'n';
    out_str[i++] = 'o';
    out_str[i++] = 'w';
    out_str[i++] = 'n';
    out_str[i++] = ')';
    out_str[i++] = '\0';
    return;
  } else if(p.length == DISTRAC_PARAC_PATH_PARSER) {
    size_t i = 0;
    out_str[i++] = '(';
    out_str[i++] = 'p';
    out_str[i++] = 'a';
    out_str[i++] = 'r';
    out_str[i++] = 's';
    out_str[i++] = 'e';
    out_str[i++] = 'r';
    out_str[i++] = ')';
    out_str[i++] = '\0';
    return;
  } else if(p.length >= DISTRAC_PARAC_PATH_MAX_LENGTH) {
    size_t i = 0;
    out_str[i++] = 'I';
    out_str[i++] = 'N';
    out_str[i++] = 'V';
    out_str[i++] = 'A';
    out_str[i++] = 'L';
    out_str[i++] = 'I';
    out_str[i++] = 'D';
    out_str[i++] = ' ';
    out_str[i++] = 'P';
    out_str[i++] = 'A';
    out_str[i++] = 'T';
    out_str[i++] = 'H';
    out_str[i++] = '\0';
    return;
  }

  for(size_t i = 0; i < DISTRAC_PARAC_PATH_MAX_LENGTH; ++i) {
    out_str[i] = distrac_parac_path_get_assignment(p, i + 1) + '0';
  }
  out_str[p.length] = '\0';
}
