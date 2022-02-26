#include <algorithm>
#include <iostream>
#include <vector>
#include <string>
#include <numeric>

using namespace std;

template <typename Iterator>
class IteratorRange {
public:
    IteratorRange(Iterator begin, Iterator end)
        : first(begin)
        , last(end)
    {
    }

    Iterator begin() const {
        return first;
    }

    Iterator end() const {
        return last;
    }

private:
    Iterator first, last;
};

template <typename Collection>
auto Head(Collection& v, size_t top) {
    return IteratorRange{ v.begin(), next(v.begin(), min(top, v.size())) };
}

struct Person {
    string name;
    int age, income;
    bool is_male;
};

vector<Person> ReadPeople(istream& input) {
    int count;
    input >> count;

    vector<Person> result(count);
    for (Person& p : result) {
        char gender;
        input >> p.name >> p.age >> p.income >> gender;
        p.is_male = gender == 'M';
    }

    return result;
}

template <typename Iter>
string FindMostName(IteratorRange<Iter> range) {
    if (range.begin() == range.end()) {
        return string();
    }

    const string* most_popular_name = &range.begin()->name;
    int count = 1;
    for (auto i = range.begin(); i != range.end(); ) {
        auto same_name_end = find_if_not(i, range.end(), [i](const Person& p) {
            return p.name == i->name;
            });
        auto cur_name_count = std::distance(i, same_name_end);
        if (cur_name_count > count) {
            count = cur_name_count;
            most_popular_name = &i->name;
        }
        i = same_name_end;
    }

    return *most_popular_name;
}

int main() {
    const vector<Person> peopleSortedByAge = [] {
        vector<Person> result = ReadPeople(cin);
        sort(begin(result), end(result), [](const Person& lhs, const Person& rhs) {
            return lhs.age < rhs.age;
            });
        return move(result);
    }();
    
    const vector<Person> peopleSortedByIncome = [&] {
        vector<Person> result(peopleSortedByAge.begin(), peopleSortedByAge.end());
        sort(result.begin(), result.end(), [](const Person& lhs, const Person& rhs) {
            return lhs.income > rhs.income;
            });
        return move(result);
    }();

    const vector<int> accumWealth = [&] {
        vector<int> result(peopleSortedByIncome.size());
        if (!peopleSortedByIncome.empty()) {
            result[0] = peopleSortedByIncome[0].income;
            for (size_t i = 1; i < peopleSortedByIncome.size(); ++i) {
                result[i] = result[i - 1] + peopleSortedByIncome[i].income;
            }
        }
        return result;
    }();

    const vector<Person> peopleSortedGender = [&] {
        vector<Person> result = peopleSortedByAge;
        sort(result.begin(), result.end(), [](const Person& lhs, const Person& rhs) {
            if (lhs.is_male == rhs.is_male) {
                return lhs.name < rhs.name;
            }

            return lhs.is_male;
            });
        return move(result);
    }();
    IteratorRange malesRange(peopleSortedGender.begin(), find_if(peopleSortedGender.begin(), peopleSortedGender.end(), [](const Person& p) {
        return !p.is_male;
        }));

    IteratorRange femaleRange(malesRange.end(), peopleSortedGender.end());

    const string MostMale = FindMostName(malesRange);
    const string MostFemale = FindMostName(femaleRange);
    

    for (string command; cin >> command; ) {
        if (command == "AGE") {
            int adult_age;
            cin >> adult_age;

            auto adult_begin = lower_bound(
                begin(peopleSortedByAge), end(peopleSortedByAge), adult_age, [](const Person& lhs, int age) {
                    return lhs.age < age;
                }
            );

            cout << "There are " << std::distance(adult_begin, end(peopleSortedByAge))
                << " adult people for maturity age " << adult_age << '\n';
        }
        else if (command == "WEALTHY") {
            int count;
            cin >> count;

            cout << "Top-" << count << " people have total income "
                << accumWealth[count - 1] << '\n';
        }
        else if (command == "POPULAR_NAME") {
            char gender;
            cin >> gender;

            string name;

            name = gender == 'M' ? MostMale : MostFemale;

            if (name.empty()) {
                cout << "No people of gender " << gender << '\n';
            }
            else {
                cout << "Most popular name among people of gender " << gender << " is "
                    << name << '\n';
            }
        }
        
    }
}