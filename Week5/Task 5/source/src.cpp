#include <algorithm>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>
#include <queue>
#include <unordered_map>
#include <unordered_set>


class BookingManager {
private:
  struct Book {
    uint64_t time = 0;
    uint32_t client_id = 0;
    uint16_t room_count = 0;
  };
  struct HotelInfoHelper {
    std::unordered_map<uint32_t, int32_t> clients;
    uint32_t rooms_count = 0;
  };

  void EraseFromHotelInfo(std::string const & hotel_name, Book const & book_to_erase) {
    auto & info = m_hotelHelper[hotel_name];
    info.rooms_count -= book_to_erase.room_count;

    auto it = info.clients.find(book_to_erase.client_id);
    if (it != info.clients.end()) {
      it->second--;
      if (it->second == 0) {
        info.clients.erase(it);
      }
    }
  }

  void ShrinkBookingDataToDay(std::string const & hotel_name) {
    auto & book_queue = m_booking[hotel_name];
    while (!book_queue.empty() && current_time - book_queue.front().time >= time_day) {
      EraseFromHotelInfo(hotel_name, book_queue.front());
      book_queue.pop_front();
    }
  }
public:
  void MakeBook(uint64_t time, std::string const & hotel_name, uint32_t client_id, uint16_t room_count) {
    time += time_offset;
    current_time = time;

    m_booking[hotel_name].push_back({time, client_id, room_count});
    m_hotelHelper[hotel_name].rooms_count += room_count;
    m_hotelHelper[hotel_name].clients[client_id]++;
  }


  uint32_t GetClients(std::string const & hotel_name) {
    ShrinkBookingDataToDay(hotel_name);

    return m_hotelHelper[hotel_name].clients.size();
  }

  uint32_t GetRooms(std::string const & hotel_name) {
    ShrinkBookingDataToDay(hotel_name);

    return m_hotelHelper[hotel_name].rooms_count;
  }

private:
  std::unordered_map<std::string, std::deque<Book>> m_booking;
  std::unordered_map<std::string, HotelInfoHelper> m_hotelHelper;

  uint64_t current_time = 0;
  const uint64_t time_offset = 1e18;
  const uint64_t time_day = 86400;
};


int main() {
  int n; std::cin >> n;
  BookingManager b;
  while (n--) {
    std::string command;
    std::cin >> command;
    if (command == "CLIENTS") {
      std::string hotel_name; std::cin >> hotel_name;
      std::cout << b.GetClients(hotel_name) << '\n';
    }
    if (command == "BOOK") {
      uint64_t time; std::cin >> time;
      std::string hotel_name; std::cin >> hotel_name;
      uint32_t client_id; std::cin >> client_id;
      uint16_t room_count; std::cin >> room_count;
      b.MakeBook(time, hotel_name, client_id, room_count);
    }
    if (command == "ROOMS") {
      std::string hotel_name; std::cin >> hotel_name;
      std::cout << b.GetRooms(hotel_name) << '\n';
    }
  }

  return 0;
}