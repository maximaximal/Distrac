#include <catch2/catch.hpp>
#include <sys/stat.h>

#include <distrac/distrac.h>
#include <distrac/headers.h>
#include <distrac/types.h>

static distrac_event_header event_headers[] = {
  distrac_event_header{ "event1", 3 },
  distrac_event_header{ "event2", 3 }
};

static distrac_property_header ev0_property_headers[]{
  distrac_property_header{ "prop1", DISTRAC_TYPE_INT16, false, 0, 0 },
  distrac_property_header{ "prop2", DISTRAC_TYPE_INT16, false, 0, 0 },
  distrac_property_header{ "prop3", DISTRAC_TYPE_INT32, false, 0, 0 }
};
static distrac_property_header ev1_property_headers[]{
  distrac_property_header{ "other-prop1", DISTRAC_TYPE_INT16, false, 0, 0 },
  distrac_property_header{ "other-prop2", DISTRAC_TYPE_INT16, false, 0, 0 },
  distrac_property_header{ "other-prop3", DISTRAC_TYPE_INT32, false, 0, 0 }
};

static distrac_property_header* property_headers[]{ ev0_property_headers,
                                                    ev1_property_headers };

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
      def->file_header.event_count = 2;
      def->event_headers = event_headers;
      def->property_headers = property_headers;
    },
    "/dev/shm/",
    output_file,
    0,
    "testbench",
    "distrac-test");

  handle.is_main_node = true;

  REQUIRE(handle.definition.events[0].size == 8);
  REQUIRE(handle.definition.events[1].size == 8);

  char ev[8] = { 'a', '#' };

  distrac_push(&handle, ev, 0);
  distrac_push(&handle, ev, 0);
  distrac_push(&handle, ev, 0);
  distrac_push(&handle, ev, 1);
  distrac_push(&handle, ev, 0);
  distrac_push(&handle, ev, 1);

  for(size_t i = 0; i < 20; ++i) {
    ++ev[0];
    distrac_push(&handle, ev, 0);
  }

  distrac_finalize(&handle, 0);
}
