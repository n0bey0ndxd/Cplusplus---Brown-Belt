#include "test_runner.h"

#include <cstddef>  // нужно для nullptr_t

using namespace std;

// Реализуйте шаблон класса UniquePtr
template <typename T>
class UniquePtr {
private:
  T * m_data;

  void DeletePtr() {
    if (m_data != nullptr) {
      delete m_data;
      m_data = nullptr;
    }
  }
public:
  UniquePtr() : m_data(nullptr) {}
  UniquePtr(T * ptr) : m_data(ptr) {}
  UniquePtr(UniquePtr&& other) : m_data(nullptr) {
    DeletePtr();
    m_data = other.Release();
  }

  UniquePtr& operator = (const UniquePtr&) = delete;
  UniquePtr(const UniquePtr&) = delete;

  UniquePtr& operator = (nullptr_t) {
    DeletePtr();
    return *this;
  }
  UniquePtr& operator = (UniquePtr&& other) {
    DeletePtr();
    m_data = other.Release();
    return *this;
  }
  ~UniquePtr() {
    DeletePtr();
  }

  T& operator * () const {
    return *m_data;
  }

  T * operator -> () const {
    return m_data;
  }

  T * Release() {
    T * ptr = m_data;
    m_data = nullptr;
    return ptr;
  }

  void Reset(T * ptr) {
    DeletePtr();
    m_data = ptr;
  }

  void Swap(UniquePtr& other) {
    T * ptr1 = other.m_data;
    other.m_data = m_data;
    m_data = ptr1;
  }

  T * Get() const {
    return m_data;
  }
};


struct Item {
  static int counter;
  int value;
  Item(int v = 0): value(v) {
    ++counter;
  }
  Item(const Item& other): value(other.value) {
    ++counter;
  }
  ~Item() {
    --counter;
  }
};

int Item::counter = 0;


void TestLifetime() {
  Item::counter = 0;
  {
    UniquePtr<Item> ptr(new Item);
    ASSERT_EQUAL(Item::counter, 1);

    ptr.Reset(new Item);
    ASSERT_EQUAL(Item::counter, 1);
  }
  ASSERT_EQUAL(Item::counter, 0);

  {
    UniquePtr<Item> ptr(new Item);
    ASSERT_EQUAL(Item::counter, 1);

    auto rawPtr = ptr.Release();
    ASSERT_EQUAL(Item::counter, 1);

    delete rawPtr;
    ASSERT_EQUAL(Item::counter, 0);
  }
  ASSERT_EQUAL(Item::counter, 0);
}

void TestGetters() {
  UniquePtr<Item> ptr(new Item(42));
  ASSERT_EQUAL(ptr.Get()->value, 42);
  ASSERT_EQUAL((*ptr).value, 42);
  ASSERT_EQUAL(ptr->value, 42);
}

/*void Test() {
  UniquePtr<Item> ptr1(new Item(1));
  UniquePtr<Item> ptr2(new Item(2));
  UniquePtr<Item> ptr3(new Item(3));
  ptr1.Swap(ptr2);
  ASSERT_EQUAL(Item::counter, 3);
  ASSERT_EQUAL(ptr1->value, 2);
  ASSERT_EQUAL(ptr2->value, 1);

  ptr1 = std::move(ptr3);
  ASSERT_EQUAL(Item::counter, 2);
  ASSERT_EQUAL(ptr1->value, 3);
  ASSERT_EQUAL(ptr3.Get(), nullptr);

  ptr3.Reset(ptr1.Release());
  ASSERT_EQUAL(Item::counter, 2);
  ASSERT_EQUAL(ptr3->value, 3);
  ASSERT_EQUAL(ptr1.Get(), nullptr);

  ptr1.Reset(nullptr);
  Item item = *ptr3;
  UniquePtr<Item> ptr4(std::move(ptr3));
  ASSERT_EQUAL(Item::counter, 3);
  ASSERT_EQUAL(ptr3.Get(), nullptr);
  ASSERT_EQUAL(ptr4->value, 3);
  //Item item1 = *ptr1;
  ptr1.Release();
  ptr1.Release();
  ptr1.Release();
}*/

int main() {
  TestRunner tr;
  RUN_TEST(tr, TestLifetime);
  RUN_TEST(tr, TestGetters);
  //RUN_TEST(tr, Test);
}
