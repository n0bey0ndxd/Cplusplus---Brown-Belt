//------------------------geom2d.hpp------------------------------------------
#include <vector>
#include <cmath>
#include <cstdint>

namespace geom2d {
class PointD {
public:
  explicit PointD(double _lat, double _lon)
          : lat(_lat), lon(_lon) {}

  double GetLat() const {
    return lat;
  }
  double GetLon() const {
    return lon;
  }
private:
  double lat;
  double lon;
};

constexpr int32_t EARTH_RADIUS_KM = 6371;
constexpr double PI = 3.1415926535;

static double ToRadians(long double degree) {
  double one_deg = PI / 180.0;
  return one_deg * degree;
}

/**
 * @brief Calculates distance on earth between 2 PointD
 */
static double CalculateDistance(PointD const & pt1, PointD const & pt2) {
  double lat1 = ToRadians(pt1.GetLat());
  double lon1 = ToRadians(pt1.GetLon());
  double lat2 = ToRadians(pt2.GetLat());
  double lon2 = ToRadians(pt2.GetLon());

  double dlon = lon2 - lon1;
  double dlat = lat2 - lat1;

  long double ans = std::pow(std::sin(dlat / 2), 2) +
                    std::cos(lat1) * std::cos(lat2) *
                    std::pow(std::sin(dlon / 2), 2);

  ans = 2 * asin(sqrt(ans));

  // Calculate the result
  ans *= EARTH_RADIUS_KM;

  //convert to meters
  ans *= 1000.0;

  return ans;
}

/**
 * @brief Calculate ratio between road distance and earth distance
 * @param dist_road road distance
 * @param dist_earth earth distance
 * @return curvature
 */
static double CalculateCurvature(uint64_t dist_road, double dist_earth) {
  return static_cast<double>(dist_road) / dist_earth;
}
} // namespace geom2d

//------------------------bus_model.hpp---------------------------------------
#include <string>
#include <memory>
#include <unordered_map>

namespace bus_model {
class Bus;
class Stop;
using BusPtr = std::shared_ptr<Bus>;
using StopPtr = std::shared_ptr<Stop>;

class Bus {
public:
  Bus(std::string _name);

  std::string_view GetName() const;
  std::vector<std::string> const & GetRoute() const;
  void SetRoute(std::vector<std::string> route);
private:
  std::string m_name;
  std::vector<std::string> m_route;
};

class Stop {
public:
  Stop(std::string _name, geom2d::PointD _point);

  std::string_view GetName() const;
  geom2d::PointD const & GetPoint() const;

  void SetDistanceBetweenStop(std::string_view stop_name, int32_t dist);
  /**
   * @brief Return distance between this stop and other otherwise -1
   * @param stop_name
   * @return dist or -1 if no dist
   */
  int32_t GetDistanceBetweenStop(std::string_view stop_name) const;
private:
  std::string m_name;
  geom2d::PointD m_point;
  std::unordered_map<std::string, int32_t> m_dist;
};
} // namespace bus_model

//------------------------bus_model.cpp---------------------------------------
namespace bus_model {
Bus::Bus(std::string _name)
        : m_name(std::move(_name)) {}

std::string_view Bus::GetName() const {
  return m_name;
}

std::vector<std::string> const & Bus::GetRoute() const {
  return m_route;
}

void Bus::SetRoute(std::vector<std::string> route) {
  m_route = std::move(route);
}

Stop::Stop(std::string _name, geom2d::PointD _point)
        : m_name(std::move(_name)), m_point(_point) {}

std::string_view Stop::GetName() const {
  return m_name;
}

geom2d::PointD const & Stop::GetPoint() const {
  return m_point;
}

void Stop::SetDistanceBetweenStop(std::string_view stop_name, int32_t dist) {
  m_dist[std::string(stop_name)] = dist;
}

int32_t Stop::GetDistanceBetweenStop(std::string_view stop_name) const {
  auto it = m_dist.find(std::string(stop_name));
  if (it != m_dist.end()) {
    return it->second;
  }

  return -1;
}
} // namespace bus_model

//------------------------transport_base.hpp---------------------------------
#include <iostream>
#include <iomanip>
#include <sstream>
#include <unordered_map>
#include <set>
#include <optional>

enum class REQUEST_TYPE {
  CREATING_BUS = 0,
  CREATING_STOP = 1,

  INFO_BUS = 3,
  INFO_STOP = 4,
};

class Request {
public:
  Request(REQUEST_TYPE const & type, std::string request_body);

  REQUEST_TYPE GetType() const;
  std::string_view GetRequestBody() const;
protected:
  REQUEST_TYPE m_type;
  std::string m_request_body;
};

class Response {
public:
  Response(std::string && responseBody);

  std::string_view GetResponseBody() const;
private:
  std::string m_response_body;
};

std::vector<Request> ReadAndPrepareRequests(std::istream & in, bool creating);
Request ParseRequest(std::string_view request_str, bool isCreating);
void PrintResponses(std::ostream & out, std::vector<Response> responses);

class TransportBase {
public:
  struct BusStat {
    std::string bus_name;
    bool is_found = false;
    int32_t stops_count = 0;
    int32_t unique_stops = 0;
    uint64_t route_length = 0;
    double curvature = 0;

    std::string ToString() const {
      std::stringstream ss;
      ss << std::setprecision(7);
      ss << "Bus " << bus_name << ": ";

      if (is_found) {
        ss << stops_count << " stops on route, "
         << unique_stops << " unique stops, "
         << route_length << " route length, "
         << curvature << " curvature";
      }
      else {
        ss << "not found";
      }


      return ss.str();
    }
  };

  struct StopStat {
    std::string stop_name;
    bool is_found = false;
    std::vector<std::string> buses;

    std::string ToString() const {
      std::stringstream ss;
      ss << "Stop " << stop_name << ": ";

      if (is_found) {
        if (buses.size()) {
          ss << "buses";
          for (std::string const & bus_name : buses) {
            ss << " " << bus_name;
          }
        }
        else {
          ss << "no buses";
        }
      }
      else {
        ss << "not found";
      }

      return ss.str();
    }
  };

  std::vector<Response> ConsumeRequests(std::vector<Request> requests);

  bus_model::BusPtr ParseBus(std::string_view str);
  bus_model::StopPtr ParseStop(std::string_view str);
  std::vector<std::string> ParseStopByDel(std::string_view stops, char del);

private:
  /**
   * @brief Add Bus with route
   */
  void AddBus(bus_model::BusPtr bus);

  /**
   * @brief Add Stop with point
   */
  void AddStop(bus_model::StopPtr stop);

  BusStat CalculateStatForBus(std::string_view bus_name) const;
  StopStat CalculateStatForStop(std::string_view stop_name) const;

  /**
   * @brief adds bus_name in all stop objects related with bus
   *
   */
  void UpdateStops(std::string_view bus_name);

private:
  std::unordered_map<std::string_view, bus_model::BusPtr> m_buses;
  std::unordered_map<std::string_view, bus_model::StopPtr> m_stops;

  std::unordered_map<std::string_view, std::set<std::string_view>> m_stop_name_and_bus_names;
};

//------------------------transport_base.cpp---------------------------------
#include <algorithm>
#include <unordered_set>

namespace {
constexpr std::string_view BUS_STR = "Bus";
}

Request::Request(REQUEST_TYPE const & type, std::string request_body)
        : m_type(type), m_request_body(request_body) {}

REQUEST_TYPE Request::GetType() const {
  return m_type;
}

std::string_view Request::GetRequestBody() const {
  return m_request_body;
}

Request ParseRequest(std::string_view request_str, bool isCreating) {
  size_t pos = request_str.find(' ');
  auto type_str = request_str.substr(0, pos);
  request_str.remove_prefix(pos + 1u);

  if (type_str == BUS_STR) {
    return Request(isCreating ?
                  REQUEST_TYPE::CREATING_BUS : REQUEST_TYPE::INFO_BUS,
                  std::string(request_str));
  }
  else {
    return Request(isCreating ?
                  REQUEST_TYPE::CREATING_STOP : REQUEST_TYPE::INFO_STOP,
                  std::string(request_str));
  }
}

Response::Response(std::string && responseBody)
        : m_response_body(std::move(responseBody)) {}

std::string_view Response::GetResponseBody() const {
  return m_response_body;
}

void PrintResponses(std::ostream & out, std::vector<Response> responses) {
  for (auto const & response : responses) {
    out << response.GetResponseBody() << "\n";
  }
}

std::vector<Request> ReadAndPrepareRequests(std::istream & in, bool creating) {
  std::string count;
  getline(in, count);
  int n = std::stoi(count);
  std::vector<std::string> reqs_str;
  while (n--) {
    std::string str;
    getline(in, str);
    reqs_str.push_back(std::move(str));
  }

  std::vector<Request> reqs;
  for (auto const & req_str : reqs_str) {
    reqs.push_back(ParseRequest(req_str, creating));
  }

  return reqs;
}

void TransportBase::AddBus(bus_model::BusPtr bus) {
  m_buses[bus->GetName()] = bus;
  UpdateStops(bus->GetName());
}

void TransportBase::AddStop(bus_model::StopPtr stop) {
  m_stops[stop->GetName()] = stop;
  m_stop_name_and_bus_names[stop->GetName()];
}

std::vector<Response> TransportBase::ConsumeRequests(std::vector<Request> requests) {
  std::vector<Response> responses;

  for (auto const & request : requests) {
    switch (request.GetType()) {
      case REQUEST_TYPE::CREATING_BUS: {
        AddBus(ParseBus(request.GetRequestBody()));
      }
        break;
      case REQUEST_TYPE::INFO_BUS: {
        auto bus = ParseBus(request.GetRequestBody());
        responses.push_back(CalculateStatForBus(bus->GetName()).ToString());
      }
        break;
      case REQUEST_TYPE::CREATING_STOP: {
        AddStop(ParseStop(request.GetRequestBody()));
      }
        break;
      case REQUEST_TYPE::INFO_STOP:
      {
        auto stop_name = request.GetRequestBody();
        responses.push_back(CalculateStatForStop(stop_name).ToString());
      }
    }
  }

  return responses;
}

TransportBase::BusStat TransportBase::CalculateStatForBus(std::string_view bus_name) const {
  auto it = m_buses.find(bus_name);
  if (it != m_buses.end()) {
    std::vector<std::string> const & route = it->second->GetRoute();
    std::unordered_set<std::string_view> s;
    long double dist_earth = 0.0;
    uint64_t dist_road = 0.0;
    bus_model::StopPtr prev_stop = nullptr;
    for (auto const & stop_name : route) {
      bus_model::StopPtr stop = m_stops.find(stop_name)->second;
      if (prev_stop) {
        int32_t di1 = prev_stop->GetDistanceBetweenStop(stop->GetName());
        if (di1 == -1) {
          dist_road += stop->GetDistanceBetweenStop(prev_stop->GetName());
        }
        else {
          dist_road += di1;
        }

        dist_earth += geom2d::CalculateDistance(prev_stop->GetPoint(), stop->GetPoint());
      }
      prev_stop = stop;
      s.insert(stop->GetName());
    }

    return {.bus_name = std::string(bus_name),
            .is_found = true,
            .stops_count = static_cast<int32_t>(route.size()),
            .unique_stops = static_cast<int32_t>(s.size()),
            .route_length = dist_road,
            .curvature = geom2d::CalculateCurvature(dist_road, dist_earth)};
  }

  return {.bus_name = std::string(bus_name),
          .is_found = false};
}

TransportBase::StopStat TransportBase::CalculateStatForStop(std::string_view stop_name) const {
  auto it = m_stop_name_and_bus_names.find(stop_name);
  if (it != m_stop_name_and_bus_names.end()) {
    std::vector<std::string> buses(it->second.begin(), it->second.end());
    return {.stop_name = std::string(stop_name),
            .is_found = true,
            .buses = std::move(buses)};
  }

  return {.stop_name = std::string(stop_name),
          .is_found = false,
          .buses = {}};
}

void TransportBase::UpdateStops(std::string_view bus_name) {
  auto bus_it = m_buses.find(bus_name);
  if (bus_it != m_buses.end()) {
    auto const & route = bus_it->second->GetRoute();
    for (std::string_view stop_name : route) {
      m_stop_name_and_bus_names[stop_name].insert(bus_name);
    }
  }
}

bus_model::BusPtr TransportBase::ParseBus(std::string_view str) {
  size_t name_end_index = str.find(':');
  std::string_view name = str.substr(0, name_end_index);
  str.remove_prefix(name_end_index + 2);

  std::vector<std::string> stops;
  if (str.find('-') == str.npos) {
    stops = ParseStopByDel(str, '>');
  }
  else {
    stops = ParseStopByDel(str, '-');
    stops.reserve(stops.size() * 2 - 1);
    for (int i = stops.size() - 2; i >= 0; i--) {
      stops.push_back(stops[i]);
    }
  }

  bus_model::BusPtr bus = std::make_shared<bus_model::Bus>(std::string(name));
  bus->SetRoute(std::move(stops));
  return bus;
}

bus_model::StopPtr TransportBase::ParseStop(std::string_view str) {
  size_t name_end_point = str.find(':');
  std::string_view name = str.substr(0, name_end_point);
  str.remove_prefix(name_end_point + 2);

  size_t coords_index = str.find(',');
  double lat = std::stod(std::string(str.substr(0, coords_index)));
  str.remove_prefix(coords_index + 2);

  size_t com_index = str.find(',');
  double lon = std::stod(std::string(str.substr(0, com_index)));

  auto stop_ptr = std::make_shared<bus_model::Stop>(std::string(name), geom2d::PointD(lat, lon));

  while (com_index != str.npos) {
    str.remove_prefix(com_index + 2);

    size_t m_index = str.find('m');
    uint32_t dist = std::stoi(std::string(str.substr(0, m_index)));
    str.remove_prefix(m_index + 5); //also delete "to "

    com_index = str.find(',');
    stop_ptr->SetDistanceBetweenStop(str.substr(0, com_index), dist);
  }

  return stop_ptr;
}

std::vector<std::string> TransportBase::ParseStopByDel(std::string_view stops, char del) {
  std::vector<std::string> res;

  size_t index = stops.find(del);
  while(index != stops.npos) {
    res.push_back(std::string(stops.substr(0, index - 1)));
    stops.remove_prefix(index + 2);
    index = stops.find(del);
  }

  res.push_back(std::string(stops.substr()));
  return res;
}

//------------------------main.cpp--------------------------------------------
#include <iostream>

int main() {
  TransportBase tb;
  PrintResponses(std::cout, std::move(tb.ConsumeRequests(ReadAndPrepareRequests(std::cin, true))));
  PrintResponses(std::cout, std::move(tb.ConsumeRequests(ReadAndPrepareRequests(std::cin, false))));
  return 0;
}