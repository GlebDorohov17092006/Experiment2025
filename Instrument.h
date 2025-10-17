#pragma once

#include <string>

class Instrument
{
private:
    std::string name;
    double error;

public:
    Instrument(const std::string &name, double error);
    Instrument();
    std::string get_name() const;
    double get_error() const;
    void set_name(const std::string &name);
    void set_error(double error);
};