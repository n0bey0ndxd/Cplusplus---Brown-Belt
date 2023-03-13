//------------------------geom2d.hpp------------------------------------------
#include <vector>
#include <cmath>

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
} // namespace geom2d

//------------------------bus_model.hpp---------------------------------------
#include <string>
#include <memory>

namespace bus_model {
class Bus;
class Stop;
using BusPtr = std::shared_ptr<Bus>;
using StopPtr = std::shared_ptr<Stop>;

std::vector<std::string> ParseStopByDel(std::string_view stops, char del);
BusPtr ParseBus(std::string_view str);
StopPtr ParseStop(std::string_view str);

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
private:
  std::string m_name;
  geom2d::PointD m_point;
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

std::vector<std::string> ParseStopByDel(std::string_view stops, char del) {
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

BusPtr ParseBus(std::string_view str) {
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

  BusPtr bus = std::make_shared<Bus>(std::string(name));
  bus->SetRoute(std::move(stops));
  return bus;
}

StopPtr ParseStop(std::string_view str) {
  size_t name_end_point = str.find(':');
  std::string_view name = str.substr(0, name_end_point);
  str.remove_prefix(name_end_point + 2);
  size_t coords_index = str.find(',');
  double lat = std::stod(std::string(str.substr(0, coords_index)));
  double lon = std::stod(std::string(str.substr(coords_index + 2)));

  return std::make_shared<Stop>(std::string(name), geom2d::PointD(lat, lon));
}
} // namespace bus_model

//------------------------transport_base.hpp---------------------------------
#include <unordered_map>
#include <sstream>
#include <iomanip>
#include <optional>

enum class REQUEST_TYPE {
  CREATING_BUS = 0,
  CREATING_STOP = 1,

  BUS = 3,
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

Request ParseRequest(std::string_view request_str, bool isCreating);

class TransportBase {
public:
  struct Info {
    int32_t stops_count;
    int32_t unique_stops;
    long double route_length;

    std::string ToString() const {
      std::stringstream ss;
      ss << std::setprecision(6);
      ss << stops_count << " stops on route, "
         << unique_stops << " unique stops, "
         << route_length << " route length\n";

      return ss.str();
    }
  };
  std::vector<Response> ConsumeRequests(std::vector<Request> requests);
  /**
   * @brief Add Bus with route
   */
  void AddBus(bus_model::BusPtr bus);

  /**
   * @brief Add Stop with point
   */
  void AddStop(bus_model::StopPtr stop);
private:
  std::optional<Info> CalculateInfoForBus(std::string_view bus_name) const;
private:
  std::unordered_map<std::string_view, bus_model::BusPtr> m_buses;
  std::unordered_map<std::string_view, bus_model::StopPtr> m_stops;
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
                   REQUEST_TYPE::CREATING_BUS : REQUEST_TYPE::BUS,
                   std::string(request_str));
  }
  else {
    return Request(REQUEST_TYPE::CREATING_STOP, std::string(request_str));
  }
}

Response::Response(std::string && responseBody)
        : m_response_body(std::move(responseBody)) {}

std::string_view Response::GetResponseBody() const {
  return m_response_body;
}

void TransportBase::AddBus(bus_model::BusPtr bus) {
  m_buses[bus->GetName()] = bus;
}

void TransportBase::AddStop(bus_model::StopPtr stop) {
  m_stops[stop->GetName()] = stop;
}

std::vector<Response> TransportBase::ConsumeRequests(std::vector<Request> requests) {
  std::vector<Response> responses;

  for (auto const & request : requests) {
    switch (request.GetType()) {
      case REQUEST_TYPE::CREATING_BUS:
      {
        auto bus = bus_model::ParseBus(request.GetRequestBody());
        AddBus(bus);
      }
        break;
      case REQUEST_TYPE::BUS:
      {
        auto bus = bus_model::ParseBus(request.GetRequestBody());
        std::optional<TransportBase::Info> info = CalculateInfoForBus(bus->GetName());
        if (info.has_value()) {
          responses.emplace_back("Bus " + std::string(bus->GetName()) + ": " + info.value().ToString());
        }
        else {
          responses.emplace_back("Bus " + std::string(bus->GetName()) + ": not found\n");
        }
      }
        break;
      case REQUEST_TYPE::CREATING_STOP:
      {
        AddStop(bus_model::ParseStop(request.GetRequestBody()));
      }
        break;
    }
  }

  return responses;
}

std::optional<TransportBase::Info> TransportBase::CalculateInfoForBus(std::string_view bus_name) const {
  auto it = m_buses.find(bus_name);
  if (it != m_buses.end()) {
    std::vector<std::string> const & route = it->second->GetRoute();
    std::unordered_set<std::string_view> s;
    long double dist = 0.0;
    bus_model::StopPtr prev_stop = nullptr;
    for (auto const & stop_name : route) {
      bus_model::StopPtr stop = m_stops.find(stop_name)->second;
      if (prev_stop) {
        dist += geom2d::CalculateDistance(prev_stop->GetPoint(), stop->GetPoint());
      }
      prev_stop = stop;
      s.insert(stop->GetName());
    }

    return {{static_cast<int32_t>(route.size()), static_cast<int32_t>(s.size()), dist}};
  }

  return std::nullopt;
}

//------------------------main.cpp--------------------------------------------
#include <iostream>

void PrintResponses(std::vector<Response> responses) {
  for (auto const & response : responses) {
    std::cout << response.GetResponseBody();
  }
}

std::vector<Request> ReadAndPrepareRequests(bool creating) {
  std::string count;
  getline(std::cin, count);
  int n = std::stoi(count);
  std::vector<std::string> reqs_str;
  while (n--) {
    std::string str;
    getline(std::cin, str);
    reqs_str.push_back(std::move(str));
  }

  std::vector<Request> reqs;
  for (auto const & req_str : reqs_str) {
    reqs.push_back(ParseRequest(req_str, creating));
  }

  return reqs;
}

int main() {
  TransportBase tb;
  PrintResponses(std::move(tb.ConsumeRequests(ReadAndPrepareRequests(true))));
  PrintResponses(std::move(tb.ConsumeRequests(ReadAndPrepareRequests(false))));
  return 0;
}