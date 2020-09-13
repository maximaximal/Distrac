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
  const char* output_file2 = "/dev/shm/distrac-tests-out2";
  if(file_exists(output_file)) {
    remove(output_file);
  }
  if(file_exists(output_file2)) {
    remove(output_file2);
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
    1,
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

  distrac_wrapper handle2 = distrac_wrapper(
    [](distrac_definition* def) {
      def->source = "";
      def->file_header.event_count = 2;
      def->event_headers = event_headers;
      def->property_headers = property_headers;
    },
    "/dev/shm/",
    output_file2,
    2,
    "testbench-2",
    "distrac-test-2");

  distrac_push(&handle2, ev, 0);
  distrac_push(&handle2, ev, 1);
  distrac_push(&handle2, ev, 0);
  distrac_push(&handle2, ev, 1);

  std::random_device rd;
  std::mt19937_64 e2(rd());
  std::uniform_int_distribution<int64_t> dist(std::llround(std::pow(2, 61)),
                                              std::llround(std::pow(2, 62)));

  int64_t* ev_int64 = reinterpret_cast<int64_t*>(ev);
  *ev_int64 = 0;
  for(size_t i = 0; i < 1'000'000; ++i) {
    *ev_int64 = dist(e2);
    distrac_push(&handle2, ev, 1);
  }

  distrac_finalize(&handle2, 10);
}
