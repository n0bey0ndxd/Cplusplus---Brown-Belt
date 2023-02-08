#include <iomanip>
#include <iostream>
#include <vector>
#include <utility>

using namespace std;

class ReadingManager {
public:
  ReadingManager()
          : user_counter_(0),
            user_page_counts_(MAX_USER_COUNT_ + 1, 0),
            count_of_users_on_page_(MAX_PAGE_COUNT + 1, 0){}

  void Read(int user_id, int page_count) {
    if (page_count == 0) {
      return;
    }

    int page_count_prev = user_page_counts_[user_id];
    if (page_count_prev != 0) {
      count_of_users_on_page_[page_count_prev]--;
    }
    else {
      user_counter_++;
    }
    count_of_users_on_page_[page_count]++;
    user_page_counts_[user_id] = page_count;
  }

  double Cheer(int user_id) const {
    if (user_page_counts_[user_id] == 0) {
      return 0;
    }
    const int user_count = GetUserCount();
    if (user_count == 1) {
      return 1;
    }

    int page_count = user_page_counts_[user_id];
    int user_amount = 0;
    for (int i = 0; i < page_count; i++) {
      user_amount += count_of_users_on_page_[i];
    }
    // По умолчанию деление целочисленное, поэтому
    // нужно привести числитель к типу double.
    // Простой способ сделать это — умножить его на 1.0.
    return (user_amount) * 1.0 / (user_count - 1);
  }

private:
  // Статическое поле не принадлежит какому-то конкретному
  // объекту класса. По сути это глобальная переменная,
  // в данном случае константная.
  // Будь она публичной, к ней можно было бы обратиться снаружи
  // следующим образом: ReadingManager::MAX_USER_COUNT.
  static const int MAX_USER_COUNT_ = 100'000;
  static const int MAX_PAGE_COUNT = 1000;

  int user_counter_;
  vector<int> user_page_counts_;
  vector<int> count_of_users_on_page_; // кол-во пользователей на странице

  int GetUserCount() const {
    return user_counter_;
  }
  void AddUser(int user_id) {
    user_counter_++;
  }
};


int main() {
  // Для ускорения чтения данных отключается синхронизация
  // cin и cout с stdio,
  // а также выполняется отвязка cin от cout
  ios::sync_with_stdio(false);
  cin.tie(nullptr);

  ReadingManager manager;

  int query_count;
  cin >> query_count;

  for (int query_id = 0; query_id < query_count; ++query_id) {
    string query_type;
    cin >> query_type;
    int user_id;
    cin >> user_id;

    if (query_type == "READ") {
      int page_count;
      cin >> page_count;
      manager.Read(user_id, page_count);
    } else if (query_type == "CHEER") {
      cout << setprecision(6) << manager.Cheer(user_id) << "\n";
    }
  }

  return 0;
}