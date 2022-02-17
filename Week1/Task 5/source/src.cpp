#include "test_runner.h"
#include "profile.h"

#include <iostream>
#include <map>
#include <string>
#include <unordered_map>
#include <list>

using namespace std;

struct Record {
    string id;
    string title;
    string user;
    int timestamp;
    int karma;
};

struct MultiIter;

bool operator== (const pair<string, Record>& l, const pair<string, Record>& r) {
    return l.first == r.first;
}

class Database {
public:
    bool Put(const Record& record) {
        auto it = storage.insert({ {record.id, record}, {} });
        if (it.second) {
            const Record* pRec = &it.first->first.second;
            auto ts = ts_map.insert(make_pair(record.timestamp, pRec));
            auto krm = krm_map.insert(make_pair(record.karma, pRec));
            auto user = user_map.insert(make_pair(record.user, pRec));

            storage[{record.id, {}}] = { ts, krm, user };
        }

        return it.second;
    }

    const Record* GetById(const string& id) const {
        auto it = storage.find({ id, {} });
        if (it != storage.end()) {
            return &it->first.second;
        }

        return nullptr;
    }

    bool Erase(const string& id) {
        /*MultiIter mult = storage[{id, {}}];*/
        auto it = storage.find({ id, {} });
        if (it != storage.end()) {
            MultiIter mult = move(it->second);
            ts_map.erase(mult._ts);
            krm_map.erase(mult._krm);
            user_map.erase(mult._user);
            storage.erase(it);

            return true;
        }

        return false;
    }

    template <typename Callback>
    void RangeByTimestamp(int low, int high, Callback callback) const {
        auto it = ts_map.lower_bound(low);

        while (it != ts_map.end() && it->first <= high && callback(*(it->second))) {
            it++;
        }
    }

    template <typename Callback>
    void RangeByKarma(int low, int high, Callback callback) const {
        auto it = krm_map.lower_bound(low);

        while (it != krm_map.end() && it->first <= high && callback(*(it->second))) {
            it++;
        }
    }

    template <typename Callback>
    void AllByUser(const string& user, Callback callback) const {
        auto range = user_map.equal_range(user);
        auto it = range.first;
        while (it != range.second && callback(*(it->second))) {
            it++;
        }
    }

private:
    multimap<int, const Record*> ts_map;
    multimap<int, const Record*> krm_map;
    multimap<string, const Record*> user_map;

    struct MultiIter {
        multimap<int, const Record*>::iterator _ts;
        multimap<int, const Record*>::iterator _krm;
        multimap<string, const Record*>::iterator _user;
    };

    struct HasherForRec {
        size_t operator() (const pair<string, Record>& pr) const {
            hash<string> h;
            return h(pr.first);
        }
    };

    unordered_map<pair<string, Record>, MultiIter, HasherForRec> storage;
};

void TestRangeBoundaries() {
    const int good_karma = 1000;
    const int bad_karma = -10;

    Database db;
    db.Put({ "id1", "Hello there", "master", 1536107260, good_karma });
    db.Put({ "id2", "O>>-<", "general2", 1536107260, bad_karma });

    int count = 0;
    db.RangeByKarma(bad_karma, good_karma, [&count](const Record&) {
        ++count;
        return true;
        });

    ASSERT_EQUAL(2, count);
}

void TestSameUser() {
    Database db;
    db.Put({ "id1", "Don't sell", "master", 1536107260, 1000 });
    db.Put({ "id2", "Rethink life", "master", 1536107260, 2000 });

    int count = 0;
    db.AllByUser("master", [&count](const Record&) {
        ++count;
        return true;
        });

    ASSERT_EQUAL(2, count);
}

void TestReplacement() {
    const string final_body = "Feeling sad";

    Database db;
    db.Put({ "id", "Have a hand", "not-master", 1536107260, 10 });
    db.Erase("id");
    db.Put({ "id", final_body, "not-master", 1536107260, -10 });

    auto record = db.GetById("id");
    ASSERT(record != nullptr);
    ASSERT_EQUAL(final_body, record->title);
}

void MyTest() {
    const string final_body = "Feeling sad";

    Database db;
    string id = "1";
    std::vector<Record> v;
    for (size_t i = 0u; i < 100'000; i++) {
        v.push_back( {id + "1", "Have a hand", "not-master", 1536107260, 10} );
    }

    {
        LOG_DURATION("PUT");
        for (const auto& i : v) {
            db.Put(i);
        }
    }
    {
        LOG_DURATION("ERASE");
        for (const auto& i : v) {
            db.Erase(i.id);
        }
    }
}

int main() {
    TestRunner tr;
    RUN_TEST(tr, TestRangeBoundaries);
    RUN_TEST(tr, TestSameUser);
    RUN_TEST(tr, TestReplacement);
    RUN_TEST(tr, MyTest);
    return 0;
}