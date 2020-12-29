#ifndef DISTRAC_HEADERS_H
#define DISTRAC_HEADERS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define DISTRAC_NAME_LENGTH 32
#define DISTRAC_LONGNAME_LENGTH 255
#define DISTRAC_DESCRIPTION_LENGTH 255

#define DISTRAC_PROPERTY_ID 0

typedef struct distrac_version {
  uint8_t major;
  uint8_t minor;
  uint8_t patch;
  uint8_t tweak;
} distrac_version;

typedef struct distrac_node_header {
  int64_t node_id;
  char node_name[DISTRAC_LONGNAME_LENGTH];
  char node_hostname[DISTRAC_LONGNAME_LENGTH];
  char program_name[DISTRAC_LONGNAME_LENGTH];
  distrac_version distrac_version_field;
  int64_t offset_ns;
} distrac_node_header;

typedef struct distrac_property_header {
  char name[DISTRAC_NAME_LENGTH];
  uint8_t datatype;
  bool enable_after_constraint;
  uint8_t after_event_property;
} distrac_property_header;

typedef struct distrac_event_header {
  char name[DISTRAC_NAME_LENGTH];
  char description[DISTRAC_LONGNAME_LENGTH];
  bool enable_after_constraint;
  uint8_t after_event_id;
  uint8_t property_count;
  // After property count follows the specified number of property headers.
} distrac_event_header;

typedef struct distrac_file_header {
  uint32_t distrac_trace_file_signature_bytes;
  char problem_name[DISTRAC_LONGNAME_LENGTH];
  char additional_info[DISTRAC_DESCRIPTION_LENGTH];
  char metadata[DISTRAC_DESCRIPTION_LENGTH];
  char distrac_name[DISTRAC_NAME_LENGTH];
  char distrac_description[DISTRAC_DESCRIPTION_LENGTH];
  uint64_t seconds_since_epoch_on_start;
  uint8_t event_count;
  // After event count follows the specified number of event headers.
} distrac_file_header;

typedef struct distrac_event {
  size_t size;
  uint64_t count;
} distrac_event;

typedef struct distrac_definition {
  distrac_file_header file_header;
  distrac_node_header node_header;

  distrac_event_header* event_headers;
  distrac_property_header** property_headers;

  distrac_event* events;
} distrac_definition;

#ifdef __cplusplus
}
#endif

#endif
