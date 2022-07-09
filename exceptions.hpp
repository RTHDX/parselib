#pragma once

#include <string>

namespace error {

class Error {
    std::string _msg;

public:
    Error(const std::string& msg) : _msg(msg) {}
    virtual ~Error() = default;

    virtual const std::string& message() const { return _msg; }
};


namespace lexical {

class UnexpectedLexem : public Error {
public:
    UnexpectedLexem(const std::string& msg) : Error(msg) {}
};

}
}
