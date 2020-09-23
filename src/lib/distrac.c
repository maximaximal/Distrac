#include "distrac/headers.h"
#include <assert.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <sys/sendfile.h>

#include <distrac/distrac.h>
#include <distrac/types.h>
#include <unistd.h>

#ifdef __STDC_NO_THREADS__
// C11 Threads not available, using c11threads from
// https://github.com/jtsiomb/c11thread0s
#include "c11threads.h"
#else
#include <threads.h>
#endif

typedef struct distrac_internal {
  atomic_bool pushable;
  mtx_t writer_mtx;
  struct timespec init_time;
  FILE* output_files[];
} distrac_internal;

static void
open_temp_files(distrac_handle* handle, const char* mode) {
  assert(handle);
  assert(handle->working_directory);

  size_t working_dir_strlen = strlen(handle->working_directory);
  size_t path_size = working_dir_strlen + 6;
  char* path = malloc(sizeof(char) * path_size);
  for(int i = 0; i < handle->definition.file_header.event_count; ++i) {
    snprintf(path, path_size, "%s/%d", handle->working_directory, i);

    FILE** file = &handle->internal->output_files[i];
    *file = fopen(path, mode);

    assert(*file);
  }
  free(path);
}

static void
close_temp_files(distrac_handle* handle) {
  assert(handle);

  for(int i = 0; i < handle->definition.file_header.event_count; ++i) {
    FILE** file = &handle->internal->output_files[i];
    fflush(*file);
    fclose(*file);
    *file = 0;
  }
}

static void
append_file(FILE* src, FILE* tgt) {
  fseek(src, 0L, SEEK_END);
  size_t src_size = ftell(src);
  rewind(src);
  assert(src_size <= 0x7ffff000);

  int tgt_fileno = fileno(tgt);
  assert(tgt_fileno != -1);
  int src_fileno = fileno(src);
  assert(src_fileno != -1);

  off_t bytes_sent = 0;

  while(bytes_sent < src_size) {
    ssize_t sent = sendfile(tgt_fileno, src_fileno, &bytes_sent, src_size);
    assert(sent > 0);
  }
}

static void
append_events(distrac_handle* handle, uint8_t event, FILE* tgt) {
  assert(handle);
  assert(handle->internal);
  assert(event < handle->definition.file_header.event_count);
  assert(handle->definition.events);
  assert(tgt);

  append_file(handle->internal->output_files[event], tgt);
}

static void
remove_temp_files(distrac_handle* handle) {
  assert(handle);
  assert(handle->working_directory);

  size_t working_dir_strlen = strlen(handle->working_directory);
  size_t path_size = working_dir_strlen + 6;
  char* path = malloc(sizeof(char) * path_size);
  for(int i = 0; i < handle->definition.file_header.event_count; ++i) {
    snprintf(path, path_size, "%s/%d", handle->working_directory, i);
    int del = remove(path);
    assert(!del);
  }
  free(path);
}

static int64_t
compute_time_diff_ns(struct timespec oldtime, struct timespec newtime) {
  time_t diff_seconds = newtime.tv_sec - oldtime.tv_sec;
  long diff_part_nanoseconds = newtime.tv_nsec - oldtime.tv_nsec;

  int64_t diff_ns = diff_seconds * 1000000000 + diff_part_nanoseconds;
  return diff_ns;
}

static size_t
add_8byte_padding(size_t pos, FILE* file) {
  size_t align_offset = pos & 0b00000111;
  size_t required_buffer_bytes = 8 - align_offset;
  uint8_t buffer_bytes[required_buffer_bytes];
  memset(buffer_bytes, 0, sizeof(uint8_t) * required_buffer_bytes);
  size_t wrote = fwrite(buffer_bytes, required_buffer_bytes, 1, file) *
                 required_buffer_bytes;
  assert(((pos + wrote) & 0b00000111) == 0);
  return wrote;
}

static size_t
write_file_header(distrac_handle* handle, FILE* tgt) {
  assert(handle);

  size_t wrote = 0;

  wrote += fwrite(&handle->definition.file_header,
                  sizeof(handle->definition.file_header),
                  1,
                  tgt) *
           sizeof(handle->definition.file_header);

  for(uint8_t ev = 0; ev < handle->definition.file_header.event_count; ++ev) {
    distrac_event_header* ev_header = handle->definition.event_headers + ev;
    wrote += fwrite(ev_header, sizeof(*ev_header), 1, tgt) * sizeof(*ev_header);

    size_t written_properties = fwrite(handle->definition.property_headers[ev],
                                       sizeof(distrac_property_header),
                                       ev_header->property_count,
                                       tgt);

    assert(written_properties == ev_header->property_count);
    wrote += written_properties * sizeof(distrac_property_header);
  }

  return wrote;
}

static void
init_definition(distrac_definition* def) {
  def->events = NULL;
  def->event_headers = NULL;
  def->property_headers = NULL;
  def->file_header.event_count = 0;
  strcpy(def->file_header.metadata, "(No Metadata)");
  strcpy(def->file_header.problem_name, "(No Problem Name)");
  strcpy(def->file_header.additional_info, "(No Additional Info)");
  strcpy(def->file_header.distrac_name, "unnamed");
  strcpy(def->file_header.distrac_description, "(No Distrac Description)");
  gethostname(def->node_header.node_hostname, 255);
}

static void
init_definition_events(distrac_definition* def) {
  def->events = calloc(sizeof(distrac_event), def->file_header.event_count);
  for(uint8_t ev = 0; ev < def->file_header.event_count; ++ev) {
    for(uint8_t prop = 0; prop < def->event_headers[ev].property_count;
        ++prop) {
      def->events[ev].size +=
        distrac_type_sizeof(def->property_headers[ev][prop].datatype);
    }
    assert((def->events[ev].size & 0b00000111) == 0 &&
           "All events must be 8 byte aligned");
  }
}

void
distrac_init(distrac_handle* handle,
             distrac_definition_function def_func,
             const char* working_directory,
             const char* output_path,
             distrac_id node_id,
             const char* node_name,
             const char* program_name) {
  assert(handle);
  assert(def_func);
  assert(working_directory);
  assert(output_path);

  handle->working_directory = strdup(working_directory);
  handle->output_path = strdup(output_path);

  assert(strlen(node_name) < 255);
  assert(strlen(program_name) < 255);
  strcpy(handle->definition.node_header.node_name, node_name);
  strcpy(handle->definition.node_header.program_name, program_name);
  handle->definition.node_header.node_id = node_id;
  handle->definition.file_header.distrac_trace_file_signature_bytes =
    DISTRAC_FILE_SIGNATURE;

  distrac_definition* def = &handle->definition;

  init_definition(def);
  def_func(def);

  assert(def->file_header.event_count > 0);
  assert(def->event_headers != NULL);
  assert(def->property_headers != NULL);

  init_definition_events(def);

  handle->internal =
    malloc(sizeof(distrac_internal) +
           sizeof(FILE*) * handle->definition.file_header.event_count);
  assert(handle->internal);

  int s = mtx_init(&handle->internal->writer_mtx, mtx_plain);
  assert(s == thrd_success);

  handle->is_main_node = false;

  open_temp_files(handle, "a");

  clock_gettime(CLOCK_MONOTONIC, &handle->internal->init_time);

  {
    struct timespec real_time;
    clock_gettime(CLOCK_REALTIME, &real_time);
    handle->definition.file_header.seconds_since_epoch_on_start =
      real_time.tv_sec;
  }

  atomic_store(&handle->internal->pushable, true);
}

void
distrac_push(distrac_handle* handle, void* event, uint8_t event_type) {
  assert(handle);
  assert(handle->internal);
  assert(event);

  if(!atomic_load(&handle->internal->pushable)) {
    return;
  }

  struct timespec current_time;
  clock_gettime(CLOCK_MONOTONIC, &current_time);
  int64_t ns_diff =
    compute_time_diff_ns(handle->internal->init_time, current_time);

  distrac_definition* def = &handle->definition;
  distrac_event* ev_def = &def->events[event_type];

  assert(event_type < def->file_header.event_count);
  size_t event_size = ev_def->size;

  mtx_lock(&handle->internal->writer_mtx);

  ++ev_def->count;
  fwrite(
    &ns_diff, sizeof(int64_t), 1, handle->internal->output_files[event_type]);
  fwrite(event, event_size, 1, handle->internal->output_files[event_type]);

  mtx_unlock(&handle->internal->writer_mtx);
}

void
distrac_finalize(distrac_handle* handle, int64_t offset_ns) {
  assert(handle);
  distrac_definition* def = &handle->definition;
  assert(def);

  atomic_store(&handle->internal->pushable, false);

  close_temp_files(handle);

  open_temp_files(handle, "r");

  def->node_header.offset_ns = offset_ns;

  FILE* outfile = fopen(handle->output_path, "w");
  assert(outfile);

  size_t filepos = 0;

  if(handle->is_main_node) {
    filepos += write_file_header(handle, outfile);
  }

  filepos += add_8byte_padding(filepos, outfile);

  filepos += fwrite(&handle->definition.node_header,
                    sizeof(distrac_node_header),
                    1,
                    outfile) *
             sizeof(distrac_node_header);

  uint64_t event_counts[def->file_header.event_count];
  for(uint8_t i = 0; i < def->file_header.event_count; ++i) {
    event_counts[i] = handle->definition.events[i].count;
  }
  filepos +=
    fwrite(
      event_counts, sizeof(uint64_t), def->file_header.event_count, outfile) *
    sizeof(uint64_t);

  fflush(outfile);

  // Combine all temporary files into output file.
  for(uint8_t i = 0; i < def->file_header.event_count; ++i) {
    append_events(handle, i, outfile);
  }

  fflush(outfile);
  fclose(outfile);

  close_temp_files(handle);
  remove_temp_files(handle);
}

void
distrac_destroy(distrac_handle* handle) {
  assert(handle);
  assert(handle->internal);

  free(handle->definition.events);
  free(handle->working_directory);
  free(handle->output_path);
  free(handle->internal);
}
