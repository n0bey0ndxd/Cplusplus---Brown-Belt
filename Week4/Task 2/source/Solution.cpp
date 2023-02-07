#include "Common.h"

#include <list>
#include <shared_mutex>
#include <algorithm>

using namespace std;

class LruCache : public ICache {
public:
  LruCache(shared_ptr<IBooksUnpacker> books_unpacker, const Settings& settings)
    : m_books_unpacker(books_unpacker), m_settings(settings) {
  }

  BookPtr GetBook(const string& book_name) override {
    lock_guard<mutex> lock(m_mutex);

    auto it = find_if(m_cache.begin(), m_cache.end(), [&](BookPtr const lhs) {
      return lhs->GetName() == book_name;
    });

    if (it != m_cache.end()) {
      BookPtr book = *it;
      m_cache.erase(it);
      m_cache.push_front(book);
      return book;
    }

    BookPtr book = m_books_unpacker->UnpackBook(book_name);

    if (!book) {
      return book;
    }

    size_t size_of_content = book->GetContent().size();
    if (size_of_content <= m_settings.max_memory) {
      while (!m_cache.empty() && m_cache_condition.size >= m_settings.max_memory - size_of_content) {
          auto it_last = prev(m_cache.end());
          size_t delta = (*it_last)->GetContent().size();
          m_cache.erase(it_last);
          m_cache_condition.size -= delta;
      }

      m_cache.push_front(book);
      m_cache_condition.size += size_of_content;
    }

    return book;
  }

private:
  mutex m_mutex;
  list<BookPtr> m_cache;
  shared_ptr<IBooksUnpacker> m_books_unpacker;
  Settings m_settings;

  struct {
      size_t size;
  } m_cache_condition;
};


unique_ptr<ICache> MakeCache(
    shared_ptr<IBooksUnpacker> books_unpacker,
    const ICache::Settings& settings
) {
    return make_unique<LruCache>(books_unpacker, settings);
}
