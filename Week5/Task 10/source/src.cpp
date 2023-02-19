#include "test_runner.h"

#include <iostream>
#include <iomanip>
#include <string>
#include <ctime>
#include <string_view>
#include <vector>


class Date {
public:
  static const int SECONDS_IN_DAY = 60 * 60 * 24;

  Date(int year_, int month_, int day_)
  : year(year_), month(month_), day(day_) {}

  time_t AsTimestamp() const {
    std::tm t;
    t.tm_sec   = 0;
    t.tm_min   = 0;
    t.tm_hour  = 0;
    t.tm_mday  = day;
    t.tm_mon   = month - 1;
    t.tm_year  = year - 1900;
    t.tm_isdst = 0;
    return mktime(&t);
  }

  size_t static ComputeDaysDiff(const Date& date_to, const Date& date_from) {
    const time_t timestamp_to = date_to.AsTimestamp();
    const time_t timestamp_from = date_from.AsTimestamp();
    return TimeStampToDays(timestamp_to - timestamp_from);
  }

  size_t static TimeStampToDays(time_t timestamp) {
    return timestamp / Date::SECONDS_IN_DAY;
  }

  size_t static ComputeDayIndex(const Date& start, const Date& date) {
    return ComputeDaysDiff(date, start);
  }

private:
  int year;
  int month;
  int day;
};

static const Date START_DATE = Date(2000, 1, 1);

class TimeRange {
public:
  TimeRange(Date from_, Date to_)
  : from(from_), to(to_) {}

  size_t RangeDaysCount() const {
    return Date::ComputeDaysDiff(to, from) + 1u;
  }

  Date from;
  Date to;
};

class PersonalBudget {
public:
  PersonalBudget(TimeRange const & range_to_consider)
  : start_date(range_to_consider.from) {
    spent_day_and_value.assign(Date::TimeStampToDays(range_to_consider.to.AsTimestamp()), 0.0);
    earned_day_and_value.assign(Date::TimeStampToDays(range_to_consider.to.AsTimestamp()), 0.0);
  }

  void Earn(TimeRange const & time_range, int value) {
    size_t days_count = time_range.RangeDaysCount();
    double day_income = static_cast<double>(value) / static_cast<double>(days_count);
    ForEachDayInRange(time_range, [&](size_t index){
      earned_day_and_value[index] += day_income;
    });
  }

  void Spend(TimeRange const & time_range, int value) {
    size_t days_count = time_range.RangeDaysCount();
    double day_spent = static_cast<double>(value) / static_cast<double>(days_count);
    ForEachDayInRange(time_range, [&](size_t index){
      spent_day_and_value[index] += day_spent;
    });
  }

  void ComputeIncome(TimeRange const & time_range, std::ostream & out = std::cout) {
    double total_income = 0;
    ForEachDayInRange(time_range, [&](size_t index){
      total_income += earned_day_and_value[index] - spent_day_and_value[index];
    });
    out << std::setprecision(25) << std::fixed << total_income << '\n';
  }

  void PayTax(TimeRange const & time_range, int tax_perc) {
    double tax_coefficient = 1.0 - (static_cast<double>(tax_perc) / static_cast<double>(100));
    ForEachDayInRange(time_range, [&](size_t index) {
      earned_day_and_value[index] *= tax_coefficient;
    });
  }

private:
  template<class Func>
  void ForEachDayInRange(TimeRange const & time_range, Func func) {
    size_t begin = Date::ComputeDayIndex(start_date, time_range.from);
    size_t days_count = time_range.RangeDaysCount();

    for (size_t i = begin; i < begin + days_count; i++) {
      func(i);
    }
  }

  std::vector<double> spent_day_and_value;
  std::vector<double> earned_day_and_value;
  Date const start_date;
};

Date ReadDate(std::istream & in = std::cin) {
  int y, m, d;
  std::cin >> y; std::cin.ignore(1);
  std::cin >> m; std::cin.ignore(1);
  std::cin >> d;

  return {y, m, d};
}

int main() {
  PersonalBudget pb({{2000, 1, 1}, {2100, 1, 1}});

  int n = 0;
  std::cin >> n;
  while(n--) {
    std::string command;
    std::cin >> command;
    TimeRange range{ReadDate(), ReadDate()};

    if (command == "Earn") {
      int income = 0; std::cin >> income;
      pb.Earn(range, income);
    }
    if (command == "Spend") {
      int spent = 0; std::cin >> spent;
      pb.Spend(range, spent);
    }
    if (command == "ComputeIncome") {
      pb.ComputeIncome(range);
    }
    if (command == "PayTax") {
      int tax_perc = 0; std::cin >> tax_perc;
      pb.PayTax(range, tax_perc);
    }
  }
}