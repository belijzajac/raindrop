#ifndef RAINDROP_EXCEPTION_H
#define RAINDROP_EXCEPTION_H

#include <stdexcept>
#include <string>

class Exception_File : public std::runtime_error{
public:
    explicit Exception_File(const std::string &error_what) : runtime_error(error_what.c_str()) {}
    ~Exception_File() override = default;
};

#endif //RAINDROP_EXCEPTION_H
