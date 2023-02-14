#include <iostream>
#include <string>
#include <string_view>
#include <vector>
#include <unordered_set>

class DomainManager {
private:
  std::vector<std::string> ReadDomains(std::istream & in = std::cin) {
    int count = 0;
    in >> count;
    std::vector<std::string> result;
    result.reserve(count);
    while (count--) {
      std::string domain;
      in >> domain;
      result.push_back(std::move(domain));
    }

    return result;
  }

  bool IsDomainAllowed(std::string_view dom_to_check) const {
    if (m_banned_domains_index.find(dom_to_check) != m_banned_domains_index.end()) {
      return false;
    }

    for (int i = dom_to_check.size() - 1; i > 0; i--) {
      if (dom_to_check[i] == '.') {
        std::string_view domain = dom_to_check.substr(i + 1);
        if (m_banned_domains_index.find(domain) != m_banned_domains_index.end()) {
          return false;
        }
      }
    }

    return true;
  }

public:
  void ReadBannedDomains(std::istream & in = std::cin) {
    m_banned_domains = std::move(ReadDomains(in));
    m_banned_domains_index = {m_banned_domains.begin(), m_banned_domains.end()};
  }

  void ReadToCheckDomains(std::istream & in = std::cin) {
    m_to_check_domains = std::move(ReadDomains(in));
  }

  void CheckDomains(std::ostream & out = std::cout) const {
    for (std::string_view dom_to_check : m_to_check_domains) {
        if (IsDomainAllowed(dom_to_check)) {
          out << _good;
        }
        else {
          out << _bad;
        }
    }
  }

private:
  std::string _good = "Good\n";
  std::string _bad = "Bad\n";

  std::unordered_set<std::string_view> m_banned_domains_index;
  std::vector<std::string> m_banned_domains;
  std::vector<std::string> m_to_check_domains;
};

int main() {
  DomainManager dm;
  dm.ReadBannedDomains();
  dm.ReadToCheckDomains();
  dm.CheckDomains();
  return 0;
}