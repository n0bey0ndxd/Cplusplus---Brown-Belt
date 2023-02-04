#include "Common.h"
#include "test_runner.h"

#include <sstream>

using namespace std;

class Val : public Expression {
public:
    Val(int _value) : value(_value) {}

    int Evaluate() const override {
        return value;
    }

    string ToString() const override {
        return std::to_string(value);
    }

private:
    const int value;
};

class Sign : public Expression {
public:
    Sign(std::string _sign, ExpressionPtr _left, ExpressionPtr _right)
    :
        sign(std::move(_sign)),
        left(std::move(_left)),
        right(std::move(_right))
    {}
    int Evaluate() const override {
        if (sign == "+") {
            return left->Evaluate() + right->Evaluate();
        }
        if (sign == "*") {
            return left->Evaluate() * right->Evaluate();
        }

        return 0;
    }

    string ToString() const override {
        return "(" + left->ToString() + ")" + sign + "(" + right->ToString() + ")";
    }
private:
    const std::string sign;
    ExpressionPtr left;
    ExpressionPtr right;
};

ExpressionPtr Value(int value) {
    return make_unique<Val>(value);
}
ExpressionPtr Sum(ExpressionPtr left, ExpressionPtr right) {
    return make_unique<Sign>("+", std::move(left), std::move(right));
}
ExpressionPtr Product(ExpressionPtr left, ExpressionPtr right) {
    return make_unique<Sign>("*", std::move(left), std::move(right));
}


string Print(const Expression* e) {
    if (!e) {
        return "Null expression provided";
    }
    stringstream output;
    output << e->ToString() << " = " << e->Evaluate();
    return output.str();
}

void Test() {
    ExpressionPtr e1 = Product(Value(2), Sum(Value(3), Value(4)));
    ASSERT_EQUAL(Print(e1.get()), "(2)*((3)+(4)) = 14");

    ExpressionPtr e2 = Sum(move(e1), Value(5));
    ASSERT_EQUAL(Print(e2.get()), "((2)*((3)+(4)))+(5) = 19");

    ASSERT_EQUAL(Print(e1.get()), "Null expression provided");
}

int main() {
    TestRunner tr;
    RUN_TEST(tr, Test);
    return 0;
}