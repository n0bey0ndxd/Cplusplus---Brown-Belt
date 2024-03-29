#include <algorithm>
#include <cmath>
#include <iostream>
#include <unordered_map>
#include <string>
#include <vector>
#include <set>
#include <climits>

using namespace std;

class RouteManager {
public:
  void AddRoute(int start, int finish) {
    reachable_lists_[start].insert(finish);
    reachable_lists_[finish].insert(start);
  }

  int FindNearestFinish(int start, int finish) const {
    int result = abs(start - finish);
    if (reachable_lists_.count(start) < 1) {
      return result;
    }

    const set<int>& reachable_stations = reachable_lists_.at(start);
    if (!reachable_stations.empty()) {
      result = min(
              result,
              FindClosestPathToPoint(finish, reachable_stations)
              );
    }
    return result;
  }
private:
  int FindClosestPathToPoint(int point, const set<int>& reachable_stations) const {
    int result = INT_MAX;

    auto it = reachable_stations.lower_bound(point);
    if (it != reachable_stations.begin()) {
      result = abs(point - *prev(it));
    }
    if (it != reachable_stations.end()) {
      result = min(result, abs(point - *it));
    }

    return result;
  }

  unordered_map<int, set<int>> reachable_lists_;
};


int main() {
  RouteManager routes;

  int query_count;
  cin >> query_count;

  for (int query_id = 0; query_id < query_count; ++query_id) {
    string query_type;
    cin >> query_type;
    int start, finish;
    cin >> start >> finish;
    if (query_type == "ADD") {
      routes.AddRoute(start, finish);
    } else if (query_type == "GO") {
      cout << routes.FindNearestFinish(start, finish) << "\n";
    }
  }

  return 0;
}
