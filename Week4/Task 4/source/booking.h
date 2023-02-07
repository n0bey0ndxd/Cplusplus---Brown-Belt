#pragma once

namespace RAII {

template<class T>
class Booking {
private:
  T * m_provider;
  int m_counter;

public:
  Booking(T * provider, int counter)
  : m_provider(provider) {}
  ~Booking() {
    if (m_provider != nullptr) {
      m_provider->CancelOrComplete(*this);
    }
  }

  Booking(Booking const &) = delete;
  Booking(Booking && other) : m_provider(other.m_provider), m_counter(other.m_counter) {
    other.m_provider = nullptr;
    other.m_counter = 0;
  }
  Booking & operator = (Booking const &) = delete;
  Booking & operator = (Booking && other) {
    m_provider = other.m_provider;
    m_counter = other.m_counter;
    other.m_provider = nullptr;
    other.m_counter = 0;
    return *this;
  }
};
} // namespace RAII
