#ifndef DISTRAC_DISTRAC_H
#define DISTRAC_DISTRAC_H

#include <distrac/headers.h>
#include <distrac/types.h>

#define DISTRAC_MAX_EVENT_COUNT 255
#define DISTRAC_MAX_EVENT_SIZE 254

#define DISTRAC_FILE_SIGNATURE 0xD152403

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

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

int64_t
distrac_current_time(distrac_handle* handle);

int64_t
distrac_push(distrac_handle* handle, const void* event, uint8_t event_type);

void
distrac_push_with_timestamp(distrac_handle* handle,
                            const void* event,
                            uint8_t event_type,
                            uint64_t timestamp);

void
distrac_finalize(distrac_handle* handle, int64_t offset_ns);

void
distrac_destroy(distrac_handle* handle);

#ifdef __cplusplus
}

class distrac_wrapper : public distrac_handle {
  public:
  distrac_wrapper() {
    working_directory = nullptr;
    output_path = nullptr;
  }
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

  void finalize(int64_t offset) { distrac_finalize(this, offset); }

  private:
  bool _initialized = false;
};
#endif

#endif
