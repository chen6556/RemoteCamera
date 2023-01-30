#include "Dongle/Dongle.hpp"
#include <sstream>
#include <fstream>


Dongle::Dongle()
{
    encode(_seed + get_uuid());
}

const std::string Dongle::encode(const char str[])
{
    std::ostringstream oss;
    _encoder.process_bytes(str, strlen(str));
    _encoder.get_digest(_code);
    for (int i = 0; i < 4; ++i)
    {
        oss << std::hex << _code[i];
    }
    return oss.str();
}

const std::string Dongle::encode(const std::string& str)
{
    std::ostringstream oss;
    _encoder.process_bytes(str.c_str(), str.length());
    _encoder.get_digest(_code);
    for (int i = 0; i < 4; ++i)
    {
        oss << std::hex << _code[i];
    }
    return oss.str();
}

const unsigned int* const Dongle::code() const
{
    return _code;
}

#ifdef __linux__

    const std::string Dongle::get_uuid()
    {
        char buf_ps[48] = {'\0'};
        char cmd[] = "sudo dmidecode -s system-uuid";
        FILE *f;
        if( (f = popen(cmd, "r")) != NULL)
        {
            fgets(buf_ps, 48, f);
            fgets(buf_ps, 48, f);
            pclose(f);
            f = NULL;
        }
        for (size_t i = 0; i < 48; ++i)
        {
            if (buf_ps[i] == ' ' || buf_ps[i] == '\r' || buf_ps[i] == '\n')
            {
                buf_ps[i] = '\0';
                break;
            }
        }
        return std::string(buf_ps);
    }

#elif _WIN32

    const std::string Dongle::get_uuid()
    {
        char buf_ps[48] = {'\0'};
        char cmd[] = "wmic csproduct get uuid";
        FILE *f;
        if( (f = _popen(cmd, "r")) != NULL)
        {
            fgets(buf_ps, 48, f);
            fgets(buf_ps, 48, f);
            _pclose(f);
            f = NULL;
        }
        for (size_t i = 0; i < 48; ++i)
        {
            if (buf_ps[i] == ' ' || buf_ps[i] == '\r' || buf_ps[i] == '\n')
            {
                buf_ps[i] = '\0';
                break;
            }
        }
        return std::string(buf_ps);
    }

#endif

void Dongle::write_passwd(const char path[], const char str[]) const
{
    std::ofstream outfile;
    outfile.open(path, std::ios::out);
    outfile << str;
    outfile.close();
}

std::string Dongle::read_passwd(const char path[]) const
{
    std::ifstream infile;
    infile.open(path, std::ios::in);
    char temp[64];
    infile.getline(temp, 64);
    infile.close();
    return std::string(temp);
}

const bool Dongle::verify(const char key[])
{   
    char temp[48] = {'\0'};
    for (size_t i = 0, j = 0, length = strlen(key); i < length; ++i)
    {
        if ( key[i] != '-')
        {
            temp[j++] = key[i];
        }
    }
    return encode(temp) == validate_code();
}

const bool Dongle::verify(const std::string& key)
{
    char temp[48] = {'\0'};
    for (size_t i = 0, j = 0, length = key.length(); i < length; ++i)
    {
        if ( key[i] != '-')
        {
            temp[j++] = key[i];
        }
    }
    return encode(temp) == validate_code();
}

const std::string Dongle::build_passwd(const bool fmt)
{
    std::string passwd = encode(_seed + get_uuid());
    if (fmt)
    {
        std::string temp = passwd.substr(0, 8);
        for (size_t i = 8, end = passwd.length(); i < end; i += 8)
        {
            temp.append("-").append(passwd.substr(i, 8));
        }
        return temp;
    }
    else
    {
        return passwd;
    }
}

const std::string Dongle::request_code()
{
    return get_uuid();
}

const std::string Dongle::validate_code()
{
    return encode(encode(_seed + get_uuid()));
}