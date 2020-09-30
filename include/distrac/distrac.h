#ifndef DISTRAC_DISTRAC_H
#define DISTRAC_DISTRAC_H

#include <distrac/headers.h>
#include <distrac/types.h>

#define DISTRAC_MAX_EVENT_COUNT 255
#define DISTRAC_MAX_EVENT_SIZE 255

#define DISTRAC_FILE_SIGNATURE 0xD152403

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

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

typedef struct distrac_handle {
  bool is_main_node;
  char* working_directory;
  char* output_path;
  struct distrac_internal* internal;

  distrac_definition definition;
} distrac_handle;

typedef void (*distrac_definition_function)(distrac_definition*);

void
distrac_init(distrac_handle* handle,
             distrac_definition_function def_func,
             const char* working_directory,
             const char* output_path,
             distrac_id node_id,
             const char* node_name,
             const char* program_name);

void
distrac_push(distrac_handle* handle, void* event, uint8_t event_type);

void
distrac_finalize(distrac_handle* handle, int64_t offset_ns);

void
distrac_destroy(distrac_handle* handle);

#ifdef __cplusplus
}

class distrac_wrapper : public distrac_handle {
  public:
  distrac_wrapper() {}
  ~distrac_wrapper() {
    if(_initialized)
      distrac_destroy(this);
  }

  void init(distrac_definition_function func,
            const char* working_directory,
            const char* output_path,
            distrac_id node_id,
            const char* node_name,
            const char* program_name) {
    distrac_init(this,
                 func,
                 working_directory,
                 output_path,
                 node_id,
                 node_name,
                 program_name);

    _initialized = true;
  }

  private:
  bool _initialized = false;
};
#endif

#endif
