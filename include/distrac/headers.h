#ifndef DISTRAC_HEADERS_H
#define DISTRAC_HEADERS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

typedef struct distrac_version {
  uint8_t major;
  uint8_t minor;
  uint8_t patch;
  uint8_t tweak;
} distrac_version;

typedef struct distrac_node_header {
  int64_t node_id;
  char node_name[255];
  char node_hostname[255];
  char program_name[255];
  distrac_version distrac_version;
  int64_t offset_ns;
} distrac_node_header;

typedef struct distrac_property_header {
  char name[32];
  uint8_t datatype;
  bool enable_after_constraint;
  uint8_t after_event_id;
  uint8_t after_event_property;
} distrac_property_header;

typedef struct distrac_event_header {
  char name[32];
  uint8_t property_count;
  // After property count follows the specified number of property headers.
} distrac_event_header;

typedef struct distrac_file_header {
  uint32_t distrac_trace_file_signature_bytes;
  char problem_name[255];
  char additional_info[255];
  char metadata[255];
  uint64_t seconds_since_epoch_on_start;
  uint8_t event_count;
  // After event count follows the specified number of event headers.
} distrac_file_header;

#ifdef __cplusplus
}
#endif

#endif
