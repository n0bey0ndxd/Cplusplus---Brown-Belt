void TestSimple() {
  {
    std::string s = "a.b.c";
    auto ans = Split(s, ".");
    ASSERT_EQUAL(ans.size(), 3);
    ASSERT_EQUAL(ans.front(), "a");
  }
  {
    std::string s = "a.b.c";
    auto exp = Split(s, ".");
    Domain d(s);
    auto range = d.GetReversedParts();
    auto it_a = range.begin();
    for (auto it_e = exp.rbegin(); it_e != exp.rend(); it_e++, it_a++) {
      ASSERT_EQUAL(*it_e, *it_a);
    }
  }
  {
    std::string s = "a.b";
    Domain d(s);
    Domain d_("c.a.b");
    ASSERT_EQUAL(IsSubdomain(d, d), true);
    ASSERT_EQUAL(IsSubdomain(d, d_), false);
  }
  {
    const vector<Domain> v_test{Domain("test.com"), Domain("a.test.com")};
    bool test_t = DomainChecker(v_test.begin(), v_test.end()).IsSubdomain(Domain("b.test.com"));
    ASSERT_EQUAL(test_t, true);
  }
  {
    std::string str_b = "1\n"
                        "a.b.c\n";
    std::string str_c = "2\n"
                        "b.c\n"
                        "a.b\n";

    stringstream in_b(str_b);
    stringstream in_c(str_c);
    const vector<Domain> banned_domains = ReadDomains(in_b);
    const vector<Domain> domains_to_check = ReadDomains(in_c);
    auto res = CheckDomains(banned_domains, domains_to_check);
    ASSERT_EQUAL(res.front(), true);
    ASSERT_EQUAL(res.back(), true);
  }
  {
    std::string str_b = "1\n"
                      "b.c\n";
    std::string str_c = "2\n"
                      "b.c\n"
                      "a.d\n";

    stringstream in_b(str_b);
    stringstream in_c(str_c);
    const vector<Domain> banned_domains = ReadDomains(in_b);
    const vector<Domain> domains_to_check = ReadDomains(in_c);
    std::string exp = "Bad\n"
                      "Good\n";

    stringstream out;
    PrintCheckResults(CheckDomains(banned_domains, domains_to_check), out);
    ASSERT_EQUAL(exp, out.str());
  }
  {
    std::string str = "2\n"
                      "a.b.c\n"
                      "b.c.d.b\n";

    stringstream in(str);
    auto ans = ReadDomains(in);
    ASSERT_EQUAL(ans.front().GetPartCount(), 3);
    ASSERT_EQUAL(ans.back().GetPartCount(), 4);
  }
}

int main() {
  TestRunner tr;
  RUN_TEST(tr, TestSimple);

  const vector<Domain> banned_domains = ReadDomains();
  const vector<Domain> domains_to_check = ReadDomains();
  PrintCheckResults(CheckDomains(banned_domains, domains_to_check));
  return 0;
}
