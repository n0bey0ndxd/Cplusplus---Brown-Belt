#include <strstream>

std::ostream& operator<<(std::ostream& stream, const AgeStats& stats) {
  return stream << "AgeStats:\n"
                << "Total: " << stats.total << '\n'
                << "Females: " << stats.females << '\n'
                << "Males: " << stats.males << '\n'
                << "Females_Emp: " << stats.employed_females << '\n'
                << "Females_UnEmp: " << stats.unemployed_females << '\n'
                << "Males_Emp: " << stats.employed_males << '\n'
                << "Males_UnEmp: " << stats.unemployed_males << '\n';
}

bool operator==(std::vector<Person> const & lhs, std::vector<Person> const & rhs) {
  if (lhs.size() != rhs.size()) {
    return false;
  }
  for (int i = 0; i < lhs.size(); i++) {
    if (!(lhs[i] == rhs[i])) {
      return false;
    }
  }

  return true;
}

void TestReadPersons() {
  std::string const str = R"(
  4
  41 1 1
  20 1 0
  33 0 1
  5 0 0
  )";

  std::vector<Person> expected = {
          {41, Gender::MALE, true},
          {20, Gender::MALE, false},
          {33, Gender::FEMALE, true},
          {5, Gender::FEMALE, false}
  };

  stringstream istream_pers(str);
  auto answer = ReadPersons(istream_pers);
  ASSERT_EQUAL(answer, expected);
}

void TestMedianAge() {
  std::string const str = R"(
  16
  41 1 1
  43 1 1
  22 1 1
  88 1 1
  63 1 0
  13 1 0
  34 1 0
  44 1 0
  35 0 1
  23 0 1
  39 0 1
  39 0 1
  10 0 0
  11 0 0
  9 0 0
  51 0 0
  )";

  int expected = 39;
  stringstream istream_pers(str);
  AgeStats answer = ComputeStats(ReadPersons(istream_pers));
  ASSERT_EQUAL(answer.total, expected);
}

void TestMedianAgeFemales() {
  std::string const str = R"(
  16
  41 1 1
  49 1 1
  22 1 1
  88 1 1
  61 1 0
  99 1 0
  54 1 0
  44 1 0
  35 0 1
  23 0 1
  39 0 1
  39 0 1
  10 0 0
  11 0 0
  9 0 0
  51 0 0
  )";

  int expected = 35;
  stringstream istream_pers(str);
  AgeStats answer = ComputeStats(ReadPersons(istream_pers));
  ASSERT_EQUAL(answer.females, expected);
}

void TestMedianAgeMales() {
  std::string const str = R"(
  16
  41 1 1
  49 1 1
  22 1 1
  88 1 1
  61 1 0
  99 1 0
  54 1 0
  44 1 0
  31 0 1
  21 0 1
  32 0 1
  39 0 1
  51 0 0
  9 0 0
  11 0 0
  )";

  int expected = 31;
  stringstream istream_pers(str);
  AgeStats answer = ComputeStats(ReadPersons(istream_pers));
  ASSERT_EQUAL(answer.females, expected);
}

void TestMedianAgeEmployedFemales() {
  std::string const str = R"(
  16
  41 1 1
  41 1 1
  41 1 1
  22 1 0
  23 1 0
  24 1 0
  27 1 0
  33 0 1
  32 0 1
  38 0 1
  39 0 1
  15 0 0
  12 0 0
  83 0 0
  51 0 0
  )";

  int expected_emp = 38;
  int expected_unemp = 51;
  stringstream istream_pers(str);
  AgeStats answer = ComputeStats(ReadPersons(istream_pers));
  ASSERT_EQUAL(answer.employed_females, expected_emp);
  ASSERT_EQUAL(answer.unemployed_females, expected_unemp);
};

void TestMedianAgeEmployedMales() {
  std::string const str = R"(
  16
  40 1 1
  42 1 1
  41 1 1
  63 1 1
  23 1 0
  23 1 0
  24 1 0
  24 1 0
  33 0 1
  32 0 1
  38 0 1
  39 0 1
  12 0 0
  83 0 0
  51 0 0
  )";

  int expected_emp = 42;
  int expected_unemp = 24;
  stringstream istream_pers(str);
  AgeStats answer = ComputeStats(ReadPersons(istream_pers));
  ASSERT_EQUAL(answer.employed_males, expected_emp);
  ASSERT_EQUAL(answer.unemployed_males, expected_unemp);
}

void TestAgeStats() {
  std::string const str = R"(
  16
  40 1 1
  44 1 1
  48 1 1
  60 1 1
  20 1 0
  23 1 0
  24 1 0
  23 1 0
  33 0 1
  32 0 1
  38 0 1
  33 0 1
  15 0 0
  17 0 0
  89 0 0
  55 0 0
  )";


  AgeStats expected = {
          .total = 33,
          .females = 33,
          .males = 40,
          .employed_females = 33,
          .unemployed_females = 55,
          .employed_males = 48,
          .unemployed_males = 23
  };

  stringstream istream_pers(str);
  AgeStats answer = ComputeStats(ReadPersons(istream_pers));
  ASSERT_EQUAL(answer, expected);
}

void TestZeros() {
  std::string str = "";
  stringstream istream_pers(str);
  ComputeStats(ReadPersons(istream_pers));
}

void TestThree() {
  std::string const str = R"(
  3
  41 1 1
  42 1 1
  43 1 1
  )";
  stringstream istream_pers(str);
  auto answer = ComputeStats(ReadPersons(istream_pers));
  ASSERT_EQUAL(answer.total, 42);
}

void TestPrintStats() {
  std::string const str = R"(
  16
  40 1 1
  44 1 1
  48 1 1
  60 1 1
  20 1 0
  23 1 0
  24 1 0
  23 1 0
  33 0 1
  32 0 1
  38 0 1
  33 0 1
  15 0 0
  17 0 0
  89 0 0
  55 0 0
  )";

  std::string expected = "Median age = 33\nMedian age for females = 33\nMedian age for males = 40\nMedian age for employed females = 33\nMedian age for unemployed females = 55\nMedian age for employed males = 48\nMedian age for unemployed males = 23\n";
  stringstream ostream_pers;

  stringstream istream_pers(str);
  AgeStats answer = ComputeStats(ReadPersons(istream_pers));
  PrintStats(answer, ostream_pers);
  std::string result = ostream_pers.str();
  ASSERT_EQUAL(expected, result);
}

int main() {
  TestRunner tr;
  RUN_TEST(tr, TestReadPersons);
  RUN_TEST(tr, TestMedianAge);
  RUN_TEST(tr, TestMedianAgeFemales);
  RUN_TEST(tr, TestMedianAgeMales);
  RUN_TEST(tr, TestMedianAgeEmployedFemales);
  RUN_TEST(tr, TestMedianAgeEmployedMales);
  RUN_TEST(tr, TestAgeStats);
  RUN_TEST(tr, TestZeros);
  RUN_TEST(tr, TestThree);
  RUN_TEST(tr, TestPrintStats);
  return 0;
}