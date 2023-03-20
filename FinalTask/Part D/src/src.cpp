//------------------------json.hpp--------------------------------------------
#include <istream>
#include <map>
#include <string>
#include <variant>
#include <vector>
#include <sstream>
#include <iomanip>
#include <cmath>

namespace Json {
class Node;

using Map = std::map<std::string, Node>;
using Array = std::vector<Node>;

class Node : std::variant<Array,
        Map,
        int32_t,
        double,
        bool,
        std::string> {
public:
  using variant::variant;

  void AddValue(Json::Node value_node, std::string key = "") {
    if (IsType<Array>()) {
      std::get<Array>(*this).push_back(value_node);
    }
    else if (IsType<Map>()) {
      std::get<Map>(*this)[key] = value_node;
    }
  }

  template<class T>
  bool IsType() const {
    return std::holds_alternative<T>(*this);
  }

  const auto& AsArray() const {
    return std::get<std::vector<Node>>(*this);
  }
  const auto& AsMap() const {
    return std::get<std::map<std::string, Node>>(*this);
  }
  const auto& AsInt() const {
    /*if (IsType<double>()) {
      return static_cast<int32_t>(AsDouble());
    }*/
    return std::get<int32_t>(*this);
  }
  double AsDouble() const {
    if (IsType<int32_t>()) {
      return static_cast<double>(AsInt());
    }
    return std::get<double>(*this);
  }
  bool AsBool() const {
    return std::get<bool>(*this);
  }
  const auto& AsString() const {
    return std::get<std::string>(*this);
  }

  std::string ToString() const {
    std::stringstream ss;
    if (IsType<Array>()) {
      ss << "[";
      auto const & this_array = this->AsArray();
      for (int32_t i = 0; i < this_array.size(); i++) {
        ss << this_array[i].ToString();
        if (i + 1 != this_array.size()) {
          ss << ", ";
        }
      }
      ss << "]";
    }
    if (IsType<Map>()) {
      ss << "{";
      auto const & this_map = this->AsMap();
      for (auto it = this_map.begin(); it != this_map.end(); it++) {
        ss << "\"" << it->first << "\":";
        ss << it->second.ToString();
        if (std::next(it) != this_map.end()) {
          ss << ", ";
        }
      }
      ss << "}";
    }
    if (IsType<int32_t>()) {
      ss << this->AsInt();
    }
    if (IsType<double>()) {
      ss << std::setprecision(7);
      ss << this->AsDouble();
    }
    if (IsType<bool>()) {
      ss << std::boolalpha;
      ss << this->AsBool();
    }
    if (IsType<std::string>()) {
      ss << "\"" << this->AsString() << "\"";
    }

    return ss.str();
  }
};

class Document {
public:
  explicit Document(Node root);

  const Node& GetRoot() const;

private:
  Node root;
};

Document Load(std::istream& input);

}

//------------------------json.cpp--------------------------------------------

namespace Json {

Document::Document(Node root) : root(std::move(root)) {
}

const Node& Document::GetRoot() const {
  return root;
}

Node LoadNode(std::istream& input);

Node LoadArray(std::istream& input) {
  std::vector<Node> result;

  for (char c; input >> c && c != ']'; ) {
    if (c != ',') {
      input.putback(c);
    }
    result.push_back(LoadNode(input));
  }

  return Node(std::move(result));
}

Node LoadNumber(std::istream& input) {
  bool isNegative = false;
  if (input.peek() == '-') {
    input.ignore(1);
    isNegative = true;
  }

  int32_t result = 0;
  while (isdigit(input.peek())) {
    result *= 10;
    result += input.get() - '0';
  }

  if (input.peek() == '.') {
    input.ignore(1); // pass '.'
    double d_result = result;
    double multiplier = 0.1;
    while (isdigit(input.peek())) {
      d_result += (input.get() - '0') * multiplier;
      multiplier /= 10.0;
    }

    return Node((isNegative ? -1.0 : 1.0) * d_result);
  }

  return Node((isNegative ? -1 : 1) * result);
}

Node LoadBoolean(std::istream& input) {
  bool result = true;
  int c = 4;

  if (input.peek() == 'f') {
    c++;
    result = false;
  }
  while(c--) {
    input.get();
  }

  return Node(result);
}

Node LoadString(std::istream& input) {
  std::string line;
  getline(input, line, '"');
  return Node(std::move(line));
}

Node LoadDict(std::istream& input) {
  std::map<std::string, Node> result;

  for (char c; input >> c && c != '}'; ) {
    if (c == ',') {
      input >> c;
    }

    std::string key = LoadString(input).AsString();
    input >> c;
    result.emplace(std::move(key), LoadNode(input));
  }

  return Node(std::move(result));
}

Node LoadNode(std::istream& input) {
  char c;
  input >> c;

  if (c == '[') {
    return LoadArray(input);
  } else if (c == '{') {
    return LoadDict(input);
  } else if (c == '"') {
    return LoadString(input);
  } else if (c == 't' || c == 'f') {
    input.putback(c);
    return LoadBoolean(input);
  } else {
    input.putback(c);
    return LoadNumber(input);
  }
}

Document Load(std::istream& input) {
  return Document{LoadNode(input)};
}

}

//------------------------geom2d.hpp------------------------------------------
#include <vector>
#include <cmath>
#include <cstdint>

namespace geom2d {
class PointD {
public:
  explicit PointD(double _lat, double _lon)
          : lat(_lat), lon(_lon) {}
  PointD() = default;

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
static double CalculateCurvature(int32_t dist_road, long double dist_earth) {
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
  Stop(std::string _name);

  std::string_view GetName() const;
  void SetPoint(geom2d::PointD point);
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

Stop::Stop(std::string _name)
        : m_name(std::move(_name)) {}

std::string_view Stop::GetName() const {
  return m_name;
}

void Stop::SetPoint(geom2d::PointD point) {
  m_point = point;
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

namespace{
static const std::string REQUEST_ID = "request_id";
static const std::string ERROR_MESSAGE = "error_message";
static const std::string ROUTE_LENGTH = "route_length";
static const std::string CURVATURE = "curvature";
static const std::string STOP_COUNT = "stop_count";
static const std::string UNIQUE_STOP_COUNT = "unique_stop_count";
static const std::string BUSES = "buses";
}

class Request {
public:
  enum class Type {
    BASE,
    STAT,
  };

  Request(Type const & type, Json::Node request_body);

  Type GetType() const;
  int32_t GetId() const;
  Json::Node const & GetRequestBody() const;
protected:
  Type m_type;
  Json::Node m_json;
};

class Response {
public:
  Response(Json::Node json, int32_t request_id);

  Json::Node const & GetResponseBody() const;
private:
  Json::Node m_json;
};

std::vector<Request> ParseRequestsJson(std::istream & in);

class TransportBase {
public:
  struct BusStat {
    std::string bus_name;
    bool is_found = false;
    int32_t stops_count = 0;
    int32_t unique_stop_count = 0;
    int32_t route_length = 0;
    double curvature = 0;

    Json::Node ToJson() {
      Json::Map dict;
      if (is_found) {
        dict[ROUTE_LENGTH] = Json::Node(route_length);
        dict[CURVATURE] = Json::Node(curvature);
        dict[STOP_COUNT] = Json::Node(stops_count);
        dict[UNIQUE_STOP_COUNT] = Json::Node(unique_stop_count);
      }
      else {
        dict[ERROR_MESSAGE] = Json::Node(std::string("not found"));
      }

      return Json::Node(std::move(dict));
    }
  };

  struct StopStat {
    std::string stop_name;
    bool is_found = false;
    std::vector<std::string> buses;

    Json::Node ToJson() const {
      Json::Map dict;
      if (is_found) {
        dict[BUSES] = std::vector<Json::Node>(buses.begin(), buses.end());
      }
      else {
        dict[ERROR_MESSAGE] = Json::Node(std::string("not found"));
      }

      return Json::Node(std::move(dict));
    }
  };

  std::vector<Response> ConsumeRequests(std::vector<Request> requests);

  bus_model::BusPtr ParseBus(Request const & request);
  bus_model::StopPtr ParseStop(Request const & request);
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
static const std::string ID = "id";
static const std::string TYPE = "type";
static const std::string BUS_STR = "Bus";
static const std::string BASE_REQUESTS = "base_requests";
static const std::string STAT_REQUESTS = "stat_requests";
static const std::string NAME = "name";
static const std::string STOPS = "stops";
static const std::string IS_ROUNDTRIP = "is_roundtrip";
static const std::string ROAD_DISTANCES = "road_distances";
static const std::string LONGITUDE = "longitude";
static const std::string LATITUDE = "latitude";
}

Request::Request(Type const & type, Json::Node json)
        : m_type(type), m_json(json) {}

Request::Type Request::GetType() const {
  return m_type;
}

int32_t Request::GetId() const {
  return m_json.AsMap().at(ID).AsInt();
}

Json::Node const & Request::GetRequestBody() const {
  return m_json;
}

Response::Response(Json::Node json, int32_t request_id)
        : m_json(std::move(json)) {
  m_json.AddValue(Json::Node(request_id), REQUEST_ID);
}

Json::Node const & Response::GetResponseBody() const {
  return m_json;
}

void PrintResponses(std::ostream & out, std::vector<Response> responses) {
  Json::Node result_response;
  for (auto const & response : responses) {
    result_response.AddValue(response.GetResponseBody());
  }
  out << result_response.ToString();
}

std::vector<Request> ParseRequestsJson(std::istream & in) {
  using namespace Json;
  Document document = Load(in);
  auto const & requests_dict = document.GetRoot().AsMap();
  std::vector<Request> requests;

  auto ParseRequestsFunc = [](std::string_view requests_key, Request::Type request_type,
                              Map _requests_dict, std::vector<Request> & _requests)
  -> void {
    Node const & base_node_req = _requests_dict.at(std::string(requests_key));
    std::vector<Node> const & req_arr = base_node_req.AsArray();
    for (Node const & req : req_arr) {
      _requests.emplace_back(request_type, req);
    }
  };

  ParseRequestsFunc(BASE_REQUESTS, Request::Type::BASE, requests_dict, requests);
  ParseRequestsFunc(STAT_REQUESTS, Request::Type::STAT, requests_dict, requests);

  return requests;
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
  for (Request const & request : requests) {
    switch (request.GetType()) {
      case Request::Type::BASE:
      {
        if (request.GetRequestBody().AsMap().find(TYPE)->second.AsString() == BUS_STR) {
          AddBus(ParseBus(request));
        }
        else {
          AddStop(ParseStop(request));
        }
      }
        break;
      case Request::Type::STAT:
      {
        if (request.GetRequestBody().AsMap().find(TYPE)->second.AsString() == BUS_STR) {
          responses.emplace_back(CalculateStatForBus(ParseBus(request)->GetName()).ToJson(), request.GetId());
        }
        else {
          responses.emplace_back(CalculateStatForStop(ParseStop(request)->GetName()).ToJson(), request.GetId());
        }
      }
        break;
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
    int32_t dist_road = 0.0;
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

        auto point1 = prev_stop->GetPoint();
        auto point2 = stop->GetPoint();
        dist_earth += geom2d::CalculateDistance(prev_stop->GetPoint(), stop->GetPoint());
      }
      prev_stop = stop;
      s.insert(stop->GetName());
    }

    return {.bus_name = std::string(bus_name),
            .is_found = true,
            .stops_count = static_cast<int32_t>(route.size()),
            .unique_stop_count = static_cast<int32_t>(s.size()),
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

bus_model::BusPtr TransportBase::ParseBus(Request const & request) {
  Json::Map const & req_body = request.GetRequestBody().AsMap();
  bus_model::BusPtr bus = std::make_shared<bus_model::Bus>(req_body.find(NAME)->second.AsString());

  if (auto it = req_body.find(STOPS); it != req_body.end()) {
    std::vector<std::string> route;

    std::vector<Json::Node> const & stops = it->second.AsArray();
    bool is_roundtrip = req_body.find(IS_ROUNDTRIP)->second.AsBool();

    for (Json::Node const & node : stops) {
      std::string const & stop_name = node.AsString();
      route.push_back(stop_name);
    }

    if (!is_roundtrip) {
      route.reserve(route.size() * 2 - 1);
      for (int32_t i = route.size() - 2; i >= 0; i--) {
        route.push_back(route[i]);
      }
    }

    bus->SetRoute(std::move(route));
  }

  return bus;
}

bus_model::StopPtr TransportBase::ParseStop(Request const & request) {
  Json::Map const & req_body = request.GetRequestBody().AsMap();

  bus_model::StopPtr stop =
          std::make_shared<bus_model::Stop>(req_body.find(NAME)->second.AsString());

  if (req_body.find(ID) == req_body.end()) {
    double lat = req_body.find(LATITUDE)->second.AsDouble();
    double lon = req_body.find(LONGITUDE)->second.AsDouble();
    stop->SetPoint(geom2d::PointD(lat, lon));

    if (auto it = req_body.find(ROAD_DISTANCES); it != req_body.end()) {
      Json::Map const & road_dists = it->second.AsMap();
      for (auto const & [name, dist] : road_dists) {
       stop->SetDistanceBetweenStop(name, dist.AsInt());
      }
    }
  }

  return stop;
}

//------------------------main.cpp--------------------------------------------
#include <iostream>

int main() {
  TransportBase tb;
  PrintResponses(std::cout, tb.ConsumeRequests(ParseRequestsJson(std::cin)));
  return 0;
}