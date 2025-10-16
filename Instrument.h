#pragma once

#include <string>

class Instrument
{
private:
    std::string name;
    double error;

public:
    Instrument(const std::string &name, double error);
    std::string get_name();
    double get_error();
    void set_name(const std::string &name);
    void set_error(double error);
};