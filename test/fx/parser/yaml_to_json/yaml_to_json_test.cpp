#include "fx/parser/yaml_to_json/yaml_to_json.hpp"
#include <gtest/gtest.h>
#include <yaml-cpp/yaml.h>
#include <nlohmann/json.hpp>
#include <variant>
#include "fx/result/result.hpp"

// Convert ---------------------------------------------------------------------

struct Convert : testing::Test {
  void static expect_convert_eq(const YAML::Node &yaml,
                                const nlohmann::json &expected_json) {
    nlohmann::json actual;
    auto result = fx::parser::yaml_to_json::convert(yaml, actual);
    ASSERT_TRUE(result.ok()) << result.error();
    EXPECT_EQ(expected_json, actual);
  }

  void static expect_convert_fail(const YAML::Node &yaml,
                                  const std::string &expected_error) {
    nlohmann::json actual;
    auto result = fx::parser::yaml_to_json::convert(yaml, actual);
    ASSERT_TRUE(result.failed());
    EXPECT_EQ(expected_error, result.error());
  }
};

TEST_F(Convert, EmptyRoot) {
  const YAML::Node yaml;

  expect_convert_fail(
      yaml,
      "The root YAML type is: empty. The root YAML object must be "
      "a map or an array.");
}

TEST_F(Convert, ScalarRoot) {
  const YAML::Node yaml = YAML::Load("416");

  expect_convert_fail(
      yaml,
      "The root YAML type is: scalar. The root YAML object must "
      "be a map or an array.");
}

TEST_F(Convert, MapRoot) {
  YAML::Node yaml;
  yaml["field"] = true;

  expect_convert_eq(yaml, R"({
    "field": true
  })"_json);
}

TEST_F(Convert, SquenceRoot) {
  YAML::Node yaml;
  yaml.push_back(416);

  expect_convert_eq(yaml, R"([416])"_json);
}

// ParseMap --------------------------------------------------------------------

struct ParseMap : testing::Test {
  void static expect_parse_eq(const YAML::Node &yaml,
                              const nlohmann::json &expected) {
    nlohmann::json actual;
    fx::parser::yaml_to_json::parse_map(yaml, actual);
    EXPECT_EQ(expected, actual);
  }
};

TEST_F(ParseMap, Empty) {
  const YAML::Node yaml = YAML::Load("{}");

  expect_parse_eq(yaml, R"({})"_json);
}

TEST_F(ParseMap, Scalars) {
  YAML::Node yaml;
  yaml["bool1"] = true;
  yaml["bool2"] = false;
  yaml["int1"] = 0;
  yaml["int2"] = 416;
  yaml["int3"] = -905;
  yaml["double1"] = 0.0;
  yaml["double2"] = 41.6;
  yaml["double3"] = -9.05;
  yaml["string1"] = "";
  yaml["string2"] = "hello world";

  expect_parse_eq(yaml, R"({
    "bool1": true,
    "bool2": false,
    "int1": 0,
    "int2": 416,
    "int3": -905,
    "double1": 0,
    "double2": 41.6,
    "double3": -9.05,
    "string1": "",
    "string2": "hello world"
  })"_json);
}

TEST_F(ParseMap, MixedTypes) {
  YAML::Node yaml;
  yaml["scalar"] = true;
  yaml["level"] = 0;
  yaml["array"].push_back(4);
  yaml["array"].push_back(1);
  yaml["array"].push_back(6);

  YAML::Node child_node;
  child_node["level"] = 1;
  child_node["nested"] = true;

  YAML::Node child_child_node;
  child_child_node["level"] = 2;

  child_node["child"] = child_child_node;
  yaml["child"] = child_node;

  expect_parse_eq(yaml, R"({
    "scalar": true,
    "level": 0,
    "array": [4, 1, 6],
    "child": {
      "level": 1,
      "nested": true,
      "child": {
        "level": 2
      }
    }
  })"_json);
}

// ParseSequence ---------------------------------------------------------------

struct ParseSequence : testing::Test {
  void static expect_parse_eq(const YAML::Node &yaml,
                              const nlohmann::json &expected) {
    nlohmann::json actual;
    fx::parser::yaml_to_json::parse_sequence(yaml, actual);
    EXPECT_EQ(expected, actual);
  }
};

TEST_F(ParseSequence, Empty) {
  const YAML::Node yaml = YAML::Load("[]");

  expect_parse_eq(yaml, R"([])"_json);
}

TEST_F(ParseSequence, OnlyScalarBools) {
  YAML::Node yaml;
  yaml.push_back(true);
  yaml.push_back(true);
  yaml.push_back(false);
  yaml.push_back(true);
  yaml.push_back(false);

  expect_parse_eq(yaml, R"([true, true, false, true, false])"_json);
}

TEST_F(ParseSequence, OnlyScalarInts) {
  YAML::Node yaml;
  yaml.push_back(4);
  yaml.push_back(1);
  yaml.push_back(6);
  yaml.push_back(-9);
  yaml.push_back(0);
  yaml.push_back(-5);

  expect_parse_eq(yaml, R"([4, 1, 6, -9, 0, -5])"_json);
}

TEST_F(ParseSequence, OnlyScalarDoubles) {
  YAML::Node yaml;
  yaml.push_back(4.1);
  yaml.push_back(6.0);
  yaml.push_back(0.0);
  yaml.push_back(-90.5);

  expect_parse_eq(yaml, R"([4.1, 6, 0, -90.5])"_json);
}

TEST_F(ParseSequence, OnlyScalarStrings) {
  YAML::Node yaml;
  yaml.push_back("a");
  yaml.push_back("b");
  yaml.push_back("");
  yaml.push_back("c");

  expect_parse_eq(yaml, R"(["a", "b", "", "c"])"_json);
}

TEST_F(ParseSequence, MixedScalarTypes) {
  YAML::Node yaml;
  yaml.push_back("abc");
  yaml.push_back(416);
  yaml.push_back(true);
  yaml.push_back(-90.5);

  expect_parse_eq(yaml, R"(["abc", 416, true, -90.5])"_json);
}

TEST_F(ParseSequence, NestedArrays) {
  YAML::Node yaml;
  yaml.push_back(YAML::Load("[[4,1,6], 9, 0, 5]"));

  YAML::Node child_two;
  child_two.push_back(YAML::Load("[true, false, true]"));
  child_two.push_back("ok");
  yaml.push_back(child_two);

  expect_parse_eq(yaml,
                  R"([[[4,1,6], 9, 0, 5], [[true, false, true], "ok"]])"_json);
}

TEST_F(ParseSequence, NestedSequence) {
  YAML::Node yaml;
  yaml.push_back(YAML::Load("[[4,1,6], 9, 0, 5]"));

  YAML::Node child_two;
  child_two["child"] = true;
  child_two["values"] = YAML::Load("[4, 1, 5]");
  yaml.push_back(child_two);

  expect_parse_eq(yaml, R"(
    [
      [[4,1,6], 9, 0, 5],
      {
        "child": true,
        "values": [4, 1, 5]
      }
    ]
  )"_json);
}

// ParseScalar -----------------------------------------------------------------

struct ParseScalar : testing::Test {
  template <typename T>
  void expect_parse_eq(const YAML::Node &yaml, T expected) {
    const auto variant = fx::parser::yaml_to_json::parse_scalar(yaml);

    if (std::is_same_v<T, bool>) {
      EXPECT_EQ(0, variant.index());
    } else if (std::is_same_v<T, int64_t>) {
      EXPECT_EQ(1, variant.index());
    } else if (std::is_same_v<T, double>) {
      EXPECT_EQ(2, variant.index());
    } else if (std::is_same_v<T, std::string>) {
      EXPECT_EQ(3, variant.index());
    } else {
      FAIL() << "Invalid variant type.";
    }

    std::visit(
        [&](auto value) {
          using K = std::decay_t<decltype(value)>;

          if constexpr (std::is_same_v<T, K>) {
            EXPECT_EQ(expected, value);
          } else {
            FAIL() << "Variant type different from expected.";
          }
        },
        variant);
  }
};

TEST_F(ParseScalar, BoolScalarTrue) {
  YAML::Node yaml;
  yaml = true;
  expect_parse_eq<bool>(yaml, true);
}

TEST_F(ParseScalar, BoolScalarFalse) {
  YAML::Node yaml;
  yaml = false;
  expect_parse_eq<bool>(yaml, false);
}

TEST_F(ParseScalar, IntScalarZero) {
  YAML::Node yaml;
  yaml = 0;
  expect_parse_eq<int64_t>(yaml, 0);
}

TEST_F(ParseScalar, IntScalarPositive) {
  YAML::Node yaml;
  yaml = 416;
  expect_parse_eq<int64_t>(yaml, 416);
}

TEST_F(ParseScalar, IntScalarNegative) {
  YAML::Node yaml;
  yaml = -416;
  expect_parse_eq<int64_t>(yaml, -416);
}

TEST_F(ParseScalar, DoubleScalarZero) {
  // 0 is too ambiguous to differentiate between int and double. So, always
  // parse 0 as an int (since it's first in the if branch).
  YAML::Node yaml;
  yaml = 0.0;
  expect_parse_eq<int64_t>(yaml, 0);
}

TEST_F(ParseScalar, DoubleScalarPositive) {
  YAML::Node yaml;
  yaml = 41.6;
  expect_parse_eq<double>(yaml, 41.6);
}

TEST_F(ParseScalar, DoubleScalarNegative) {
  YAML::Node yaml;
  yaml = -4.16;
  expect_parse_eq<double>(yaml, -4.16);
}

TEST_F(ParseScalar, StringScalarEmpty) {
  YAML::Node yaml;
  yaml = "";
  expect_parse_eq<std::string>(yaml, "");
}

TEST_F(ParseScalar, StringScalarNonEmpty) {
  YAML::Node yaml;
  yaml = "hello world";
  expect_parse_eq<std::string>(yaml, "hello world");
}
