#include <catch2/catch.hpp>
#include <sys/stat.h>

#include <distrac/distrac.h>
#include <distrac/headers.h>
#include <distrac/types.h>

static distrac_event_header event_headers[] = { distrac_event_header{ "event1",
                                                                      1 } };

static distrac_property_header ev0_property_headers[]{
  distrac_property_header{ "prop1", DISTRAC_TYPE_INT16, false, 0, 0 }
};

static distrac_property_header* property_headers[]{ ev0_property_headers };

static bool
file_exists(const char* filename) {
  struct stat buffer;
  int exist = stat(filename, &buffer);
  if(exist == 0)
    return true;
  else
    return false;
}

TEST_CASE("Distrac Push") {
  const char* output_file = "/dev/shm/distrac-tests-out";
  if(file_exists(output_file)) {
    remove(output_file);
  }

  distrac_wrapper handle(
    [](distrac_definition* def) {
      def->source = "";
      def->file_header.event_count = 1;
      def->event_headers = event_headers;
      def->property_headers = property_headers;
    },
    "/dev/shm/",
    output_file,
    0,
    "testbench",
    "distrac-test");

  handle.is_main_node = true;

  REQUIRE(handle.definition.events[0].size == 2);

  char ev[2] = { 'a', '#' };

  distrac_push(&handle, ev, 0);

  for(size_t i = 0; i < 20; ++i) {
    ++ev[0];
    distrac_push(&handle, ev, 0);
  }

  distrac_finalize(&handle, 0);
}
