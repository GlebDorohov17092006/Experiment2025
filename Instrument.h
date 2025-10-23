#pragma once

#include <string>
#include <vector>

class Instrument
{
protected:
    std::string name;

public:
    Instrument(const std::string &name = "");
    std::string get_name() const;
    virtual double get_error() const;
    void set_name(const std::string &name);
    virtual void set_error(double error);
    virtual void set_error(std::vector<double> error);
};
