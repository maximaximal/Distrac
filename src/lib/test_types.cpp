#include <catch2/catch.hpp>

#include <distrac/types.h>

TEST_CASE("Distrac Type handling") {
  uint8_t double_mem[sizeof(double)];
  double double_check = 1.11;

  double* double_ptr = reinterpret_cast<double*>(&double_mem[0]);

  *double_ptr = double_check;

  bool checked = false;

  distrac_memory_to_type(
    double_mem, DISTRAC_TYPE_DOUBLE, [&checked, double_check](auto&& val) {
      using T = std::decay_t<decltype(val)>;
      if constexpr(std::is_same_v<T, double>) {
        REQUIRE(double_check == val);
        checked = true;
      }
    });

  REQUIRE(checked);

  distrac_type_variant var =
    distrac_memory_to_variant(double_mem, DISTRAC_TYPE_DOUBLE);
  REQUIRE(std::holds_alternative<double>(var));
  REQUIRE(std::get<double>(var) == double_check);
}
