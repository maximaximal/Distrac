#include "distrac/analysis/node.hpp"
#include <catch2/catch.hpp>
#include <sys/stat.h>

#include <distrac/distrac.h>
#include <distrac/headers.h>
#include <distrac/types.h>

#include <distrac/analysis/event_iterator.hpp>
#include <distrac/analysis/property.hpp>
#include <distrac/analysis/tracefile.hpp>

#include <algorithm>
#include <iostream>
#include <list>
#include <numeric>
#include <vector>

std::ostream&
operator<<(std::ostream& o, std::vector<int32_t> v) {
  for(int32_t e : v) {
    o << e << " ";
  }
  return o;
}

static distrac_event_header event_headers[] = {
  distrac_event_header{ "event0", "", false, 0, 2 },
  distrac_event_header{ "event1", "", true, 0, 2 },
};

static distrac_property_header ev0_property_headers[]{
  distrac_property_header{ "prop0", DISTRAC_TYPE_INT32, false, 0 },
  distrac_property_header{ "prop1", DISTRAC_TYPE_INT32, false, 0 },
};

static distrac_property_header ev1_property_headers[]{
  distrac_property_header{ "prop0", DISTRAC_TYPE_INT32, true, 1 },
  distrac_property_header{ "prop1", DISTRAC_TYPE_INT32, false, 0 },
};

static distrac_property_header* property_headers[]{
  ev0_property_headers,
  ev1_property_headers,
};

static bool
file_exists(const char* filename) {
  struct stat buffer;
  int exist = stat(filename, &buffer);
  if(exist == 0)
    return true;
  else
    return false;
}

static std::vector<int32_t>
populate_data_vector_inorder(size_t event_count, size_t node_count) {
  std::vector<int32_t> data(event_count * node_count);
  std::iota(data.begin(), data.end(), 1);
  return data;
}

static void
produce_random_data(const char* out,
                    bool mainNode,
                    distrac_id nodeId,
                    size_t event_count,
                    const std::vector<int32_t>& data,
                    int64_t data_offset) {
  if(file_exists(out)) {
    remove(out);
  }

  distrac_wrapper handle;
  handle.init(
    [](distrac_definition* def) {
      def->file_header.event_count = 2;
      def->event_headers = event_headers;
      def->property_headers = property_headers;
    },
    "/dev/shm/",
    out,
    nodeId,
    "testbench",
    "distrac-test");

  handle.is_main_node = mainNode;

  REQUIRE(handle.definition.events[0].size == 8);

  std::uniform_int_distribution<int32_t> dist(-event_count, event_count);

  for(size_t i = 0; i < event_count; ++i) {
    distrac_push_with_timestamp(&handle,
                                data.data() + event_count * (nodeId - 1) + i,
                                0,
                                i + data_offset);

    if(nodeId > 1 && i > 1) {
      distrac_push_with_timestamp(&handle,
                                  data.data() + event_count * (nodeId - 2) + i -
                                    1,
                                  1,
                                  i + data_offset + 10);
    }
  }

  static int order = 0;
  distrac_finalize(&handle, order++ - (nodeId != 1 ? 1000 : 0));
}

static std::string
inorder_random_data(const char* outputs[], size_t count, size_t event_count) {
  auto data = populate_data_vector_inorder(count, event_count);
  for(size_t i = 0; i < count; ++i) {
    produce_random_data(
      outputs[i], i == 0, i + 1, event_count, data, i * 1'000'000'000);
  }
  return "inorder random data";
}

static std::string
reverse_random_data(const char* outputs[], size_t count, size_t event_count) {
  auto data = populate_data_vector_inorder(count, event_count);
  for(size_t i = count; i > 1; --i) {
    produce_random_data(
      outputs[i - 1], i == 1, i, event_count, data, (i - 1) * 1'000'000'000);
  }
  return "reverse random data";
}

static std::string
internal_mixup_random_data(const char* outputs[],
                           size_t count,
                           size_t event_count) {
  auto data = populate_data_vector_inorder(count, event_count);
  produce_random_data(outputs[0], true, 1, event_count, data, 0);

  for(size_t i = count - 1; i > 2; --i) {
    produce_random_data(
      outputs[i - 1], false, i, event_count, data, (i - 1) * 1'000'000'000);
  }

  produce_random_data(
    outputs[count - 1], false, count, event_count, data, count * 1'000'000'000);
  return "internal mixup random data";
}

static std::string
external_and_internal_mixup_random_data(const char* outputs[],
                                        size_t count,
                                        size_t event_count) {
  auto data = populate_data_vector_inorder(count, event_count);
  produce_random_data(
    outputs[count - 1], false, count, event_count, data, count * 1'000'000'000);

  for(size_t i = count - 1; i > 2; --i) {
    produce_random_data(
      outputs[i - 1], false, i, event_count, data, (i - 1) * 1'000'000'000);
  }

  produce_random_data(outputs[0], true, 1, event_count, data, 0);
  return "external and internal mixup random data";
}

static std::string
external_and_no_internal_mixup_random_data(const char* outputs[],
                                           size_t count,
                                           size_t event_count) {
  auto data = populate_data_vector_inorder(count, event_count);
  produce_random_data(
    outputs[count - 1], false, count, event_count, data, count * 1'000'000'000);

  for(size_t i = 1; i < count - 1; ++i) {
    produce_random_data(
      outputs[i], false, i + 1, event_count, data, i * 1'000'000'000);
  }

  produce_random_data(outputs[0], true, 1, event_count, data, 0);
  return "external and no internal mixup random data";
}

static void
combine_files(const char* outputs[], size_t count, const char* concat_file) {
  if(file_exists(concat_file)) {
    remove(concat_file);
  }

  std::stringstream cmd;
  cmd << "cat";
  for(size_t i = 0; i < count; ++i)
    cmd << " \"" << outputs[i] << "\"";
  cmd << " > \"" << concat_file << "\"";
  system(cmd.str().c_str());
}

TEST_CASE("Entry Matcher") {
  using namespace distrac;
  static std::list<std::vector<int64_t>> value_history;

  const size_t per_node_event_count = 10;
  const size_t outputs_count = 8;
  const char* outputs[outputs_count] = {
    "/dev/shm/distrac-tests-entrymatcher-out1",
    "/dev/shm/distrac-tests-entrymatcher-out7",
    "/dev/shm/distrac-tests-entrymatcher-out2",
    "/dev/shm/distrac-tests-entrymatcher-out4",
    "/dev/shm/distrac-tests-entrymatcher-out3",
    "/dev/shm/distrac-tests-entrymatcher-out5",
    "/dev/shm/distrac-tests-entrymatcher-out6",
    "/dev/shm/distrac-tests-entrymatcher-out8",
  };
  const char* combined = "/dev/shm/distrac-tests-entrymatcher-out-concat";

  auto data_generator = GENERATE(inorder_random_data,
                                 reverse_random_data,
                                 internal_mixup_random_data,
                                 external_and_internal_mixup_random_data,
                                 external_and_no_internal_mixup_random_data);
  std::string data_variant =
    data_generator(outputs, outputs_count, per_node_event_count);

  auto& current_hist_entry = value_history.emplace_back();
  current_hist_entry.reserve(per_node_event_count * outputs_count);

  for(size_t i = 0; i < outputs_count; ++i) {
    REQUIRE(file_exists(outputs[i]));
  }

  combine_files(outputs, outputs_count, combined);

  REQUIRE(file_exists(combined));

  distrac::tracefile tracefile(combined);

  REQUIRE(tracefile.node_count() == outputs_count);
  REQUIRE(tracefile.event_count(0) == per_node_event_count * outputs_count);

  for(const auto& ev0 : tracefile.filtered(tracefile::event_filter_set{ 0 })) {
    const auto& prop0 = ev0.property(1);
    current_hist_entry.emplace_back(prop0.as<int32_t>());
  }

  if(value_history.size() > 1) {
    CAPTURE(data_variant);
    REQUIRE_THAT(value_history.front(),
                 Catch::Matchers::Equals(current_hist_entry));
  }
}
