#include "test_runner.h"
#include <functional>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include <queue>

using namespace std;


struct Email {
    Email(std::string const & from_, std::string const & to_, std::string const & body_)
    :
        from(from_),
        to(to_),
        body(body_)
    {}
    Email() = default;
    string from;
    string to;
    string body;
};


class Worker {
public:
    virtual ~Worker() = default;
    virtual void Process(unique_ptr<Email> email) = 0;
    virtual void Run() {
        // только первому worker-у в пайплайне нужно это имплементировать
        throw logic_error("Unimplemented");
    }

protected:
    // реализации должны вызывать PassOn, чтобы передать объект дальше
    // по цепочке обработчиков
    void PassOn(unique_ptr<Email> email) const {
        if (m_next) {
            m_next->Process(std::move(email));
        }
    }

public:
    void SetNext(unique_ptr<Worker> next) {
        if (!m_next) {
            m_next = std::move(next);
        }
        else {
            m_next->SetNext(std::move(next));
        }
    }

private:
    unique_ptr<Worker> m_next = nullptr;
};


class Reader : public Worker {
public:
    explicit Reader(std::istream & in)
        :
            m_in(in)
        {}

    void Process(unique_ptr<Email> email) override {
        PassOn(std::move(email));
    }

    void Run() override {
        std::string from;
        std::string to;
        std::string body;

        while (getline(m_in, from)) {
            getline(m_in, to);
            getline(m_in, body);

            Process(make_unique<Email>(from, to, body));
        }
    }

private:
    std::istream & m_in;
};


class Filter : public Worker {
public:
    using Function = function<bool(const Email&)>;

    explicit Filter(Function fn)
    :
        m_fn(std::move(fn))
    {}
public:
    void Process(unique_ptr<Email> email) override {
        if (email && m_fn(*email)) {
            PassOn(std::move(email));
        }
    }

private:
    Function m_fn;
};


class Copier : public Worker {
public:
    Copier(std::string to)
    :
        m_to(std::move(to))
    {}

    void Process(unique_ptr<Email> email) {
        std::unique_ptr<Email> new_email = nullptr;
        if (email->to != m_to) {
            new_email = make_unique<Email>(email->from, m_to, email->body);
        }
        PassOn(std::move(email));
        if (new_email) {
            PassOn(std::move(new_email));
        }
    }
private:
    std::string m_to;
};


class Sender : public Worker {
public:
    Sender(std::ostream & out)
    :
        m_out(out)
    {}

    void Process(unique_ptr<Email> email) {
        m_out << email->from << '\n';
        m_out << email->to << '\n';
        m_out << email->body << '\n';

        PassOn(std::move(email));
    }

private:
    std::ostream & m_out;
};


// реализуйте класс
class PipelineBuilder {
public:
    using work_ptr = unique_ptr<Worker>;

    // добавляет в качестве первого обработчика Reader
    explicit PipelineBuilder(istream& in) {
        m_head = make_unique<Reader>(in);
    }

    // добавляет новый обработчик Filter
    PipelineBuilder& FilterBy(Filter::Function filter) {
        work_ptr worker = make_unique<Filter>(std::move(filter));
        m_head->SetNext(std::move(worker));
        return *this;
    }

    // добавляет новый обработчик Copier
    PipelineBuilder& CopyTo(string recipient) {
        work_ptr worker = make_unique<Copier>(std::move(recipient));
        m_head->SetNext(std::move(worker));
        return *this;
    }

    // добавляет новый обработчик Sender
    PipelineBuilder& Send(ostream& out) {
        work_ptr worker = make_unique<Sender>(out);
        m_head->SetNext(std::move(worker));
        return *this;
    }

    // возвращает готовую цепочку обработчиков
    unique_ptr<Worker> Build() {
        return std::move(m_head);
    }

private:
    work_ptr m_head;
};


void TestSanity() {
    string input = (
            "erich@example.com\n"
            "richard@example.com\n"
            "Hello there\n"

            "erich@example.com\n"
            "ralph@example.com\n"
            "Are you sure you pressed the right button?\n"

            "ralph@example.com\n"
            "erich@example.com\n"
            "I do not make mistakes of that kind\n"
    );
    istringstream inStream(input);
    ostringstream outStream;

    PipelineBuilder builder(inStream);
    builder.FilterBy([](const Email& email) {
        return email.from == "erich@example.com";
    });
    builder.CopyTo("richard@example.com");
    builder.Send(outStream);
    auto pipeline = builder.Build();

    pipeline->Run();

    string expectedOutput = (
            "erich@example.com\n"
            "richard@example.com\n"
            "Hello there\n"

            "erich@example.com\n"
            "ralph@example.com\n"
            "Are you sure you pressed the right button?\n"

            "erich@example.com\n"
            "richard@example.com\n"
            "Are you sure you pressed the right button?\n"
    );

    ASSERT_EQUAL(expectedOutput, outStream.str());
}

int main() {
    TestRunner tr;
    RUN_TEST(tr, TestSanity);
    return 0;
}
