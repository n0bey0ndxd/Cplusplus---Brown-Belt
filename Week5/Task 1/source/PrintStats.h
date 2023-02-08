#pragma once

void PrintStats(vector<Person> persons) {
  auto predicateGender = [](Person const & person) {
    return person.gender == Gender::FEMALE;
  };
  auto predicateEmployee = [](Person const & person) {
    return person.is_employed;
  };

  cout << "Median age = " << ComputeMedianAge(persons.begin(), persons.end()) << '\n';

  auto it = std::partition(persons.begin(), persons.end(), predicateGender);
  auto it_f = std::partition(persons.begin(), it, predicateEmployee);
  auto it_m = std::partition(it, persons.end(), predicateEmployee);

  cout << "Median age for females = " << ComputeMedianAge(persons.begin(), it) << '\n';
  cout << "Median age for males = " << ComputeMedianAge(it, persons.end()) << '\n';

  cout << "Median age for employed females = " << ComputeMedianAge(persons.begin(), it_f) << '\n';
  cout << "Median age for unemployed females = " << ComputeMedianAge(it_f, it) << '\n';

  cout << "Median age for employed males = " << ComputeMedianAge(it, it_m) << '\n';
  cout << "Median age for unemployed males = " << ComputeMedianAge(it_m, persons.end()) << '\n';
}