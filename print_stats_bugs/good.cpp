#include "test_runner.h"

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

enum class Gender {
  FEMALE,
  MALE
};

struct Person {
  int age;
  Gender gender;
  bool is_employed;
};

bool operator==(const Person& lhs, const Person& rhs) {
  return lhs.age == rhs.age
      && lhs.gender == rhs.gender
      && lhs.is_employed == rhs.is_employed;
}

ostream& operator<<(ostream& stream, const Person& person) {
  return stream << "Person(age=" << person.age 
      << ", gender=" << static_cast<int>(person.gender)
      << ", is_employed=" << boolalpha << person.is_employed << ")";
}

struct AgeStats {
  int total;
  int females;
  int males;
  int employed_females;
  int unemployed_females;
  int employed_males;
  int unemployed_males;
};

template <typename InputIt>
int ComputeMedianAge(InputIt range_begin, InputIt range_end) {
  if (range_begin == range_end) {
    return 0;
  }
  vector<typename iterator_traits<InputIt>::value_type> range_copy(
      range_begin,
      range_end
  );
  auto middle = begin(range_copy) + range_copy.size() / 2;
  nth_element(
      begin(range_copy), middle, end(range_copy),
      [](const Person& lhs, const Person& rhs) {
        return lhs.age < rhs.age;
      }
  );
  return middle->age;
}

vector<Person> ReadPersons(istream& in_stream = cin) {
  int person_count;
  in_stream >> person_count;
  vector<Person> persons;
  persons.reserve(person_count);
  for (int i = 0; i < person_count; ++i) {
    int age, gender, is_employed;
    in_stream >> age >> gender >> is_employed;
    Person person{
        age,
        static_cast<Gender>(gender),
        is_employed == 1
    };
    persons.push_back(person);
  }
  return persons;
}

AgeStats ComputeStats(vector<Person> persons) {
  //                 persons
  //                |       |
  //          females        males
  //         |       |      |     |
  //      empl.  unempl. empl.   unempl.

  auto females_end = partition(
      begin(persons), end(persons),
      [](const Person& p) {
        return p.gender == Gender::FEMALE;
      }
  );
  auto employed_females_end = partition(
      begin(persons), females_end,
      [](const Person& p) {
        return p.is_employed;
      }
  );
  auto employed_males_end = partition(
      females_end, end(persons),
      [](const Person& p) {
        return p.is_employed;
      }
  );

  return {
       ComputeMedianAge(begin(persons), end(persons)),
       ComputeMedianAge(begin(persons), females_end),
       ComputeMedianAge(females_end, end(persons)),
       ComputeMedianAge(begin(persons),  employed_females_end),
       ComputeMedianAge(employed_females_end, females_end),
       ComputeMedianAge(females_end, employed_males_end),
       ComputeMedianAge(employed_males_end, end(persons))
  };
}

void PrintStats(const AgeStats& stats,
                ostream& out_stream = cout) {
  out_stream << "Median age = "
             << stats.total              << endl
             << "Median age for females = "
             << stats.females            << endl
             << "Median age for males = "
             << stats.males              << endl
             << "Median age for employed females = "
             << stats.employed_females   << endl
             << "Median age for unemployed females = "
             << stats.unemployed_females << endl
             << "Median age for employed males = "
             << stats.employed_males     << endl
             << "Median age for unemployed males = "
             << stats.unemployed_males   << endl;
}


void TestComputeMedianAgeEmpty() {
    vector<Person> persons;
    try {
        int computed_age = ComputeMedianAge(persons.begin(), persons.end());
        ASSERT_EQUAL(computed_age, 0);
    } catch (...) {
        ASSERT(false);
    }
}

void TestComputeMedianAgeMedian() {
    vector<Person> persons = {
        {
            .age = 0,
            .gender = Gender::FEMALE,
            .is_employed = true
        },
        {
            .age = 5,
            .gender = Gender::FEMALE,
            .is_employed = true
        },
        {
            .age = 10,
            .gender = Gender::FEMALE,
            .is_employed = true
        }
    };
    int computed_age = ComputeMedianAge(persons.begin(), persons.end());
    ASSERT_EQUAL(computed_age, 5);
}

void TestReadPersons() {
    stringstream ss;
    ss << "1\n";
    ss << "10 0 1";
    auto persons = ReadPersons(ss);
    ASSERT_EQUAL(persons.size(), 1u);
    Person expected{
        .age = 10,
        .gender = Gender::FEMALE,
        .is_employed = true
    };
    ASSERT_EQUAL(persons[0], expected);
}

void TestComputeStats() {
    vector<Person> persons = {
        {20, Gender::FEMALE, 0},
        {25, Gender::FEMALE, 1},
        {30, Gender::MALE, 0},
        {35, Gender::MALE, 1},
    };

    auto stats = ComputeStats(persons);
    ASSERT_EQUAL(stats.total, 30);
    ASSERT_EQUAL(stats.females, 25);
    ASSERT_EQUAL(stats.unemployed_females, 20);
    ASSERT_EQUAL(stats.employed_females, 25);
    ASSERT_EQUAL(stats.males, 35);
    ASSERT_EQUAL(stats.unemployed_males, 30);
    ASSERT_EQUAL(stats.employed_males, 35);
}

void TestPrintStats() {
    stringstream out;
    AgeStats stats{
        .total = 1,
        .females = 2,
        .males = 3,
        .employed_females = 4,
        .unemployed_females = 5,
        .employed_males = 6,
        .unemployed_males = 7,
    };

    PrintStats(stats, out);
    stringstream expected;
    expected << "Median age = 1" << endl
             << "Median age for females = 2" << endl
             << "Median age for males = 3" << endl
             << "Median age for employed females = 4" << endl
             << "Median age for unemployed females = 5" << endl
             << "Median age for employed males = 6" << endl
             << "Median age for unemployed males = 7" << endl;
    
    ASSERT_EQUAL(out.str(), expected.str());
}

int main() {
    TestRunner tr;
    RUN_TEST(tr, TestComputeMedianAgeEmpty);
    RUN_TEST(tr, TestComputeMedianAgeMedian);
    RUN_TEST(tr, TestReadPersons);
    RUN_TEST(tr, TestComputeStats);
    RUN_TEST(tr, TestPrintStats);

    // PrintStats(ComputeStats(ReadPersons()));
    return 0;
}
