#pragma once
#include <iostream>
#include <string>
#include <boost/uuid/detail/md5.hpp>


class Dongle
{
private:
    boost::uuids::detail::md5 _encoder;
    unsigned int _code[4];
    const std::string _seed = "0abc12-3defg4-5678hi";

public:
    Dongle();

    const std::string encode(const char str[]);
    const std::string encode(const std::string& str);
    const unsigned int* const code() const;
    static const std::string get_uuid();

    const std::string build_passwd(const bool fmt = true);
    const std::string validate_code();
    const std::string request_code();

    void write_passwd(const char path[], const char str[]) const;
    std::string read_passwd(const char path[]) const;

    const bool verify(const char key[]);
    const bool verify(const std::string& key);
};