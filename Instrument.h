#pragma once

#include <string>
//#include <vector>

class Instrument
{
protected:
    std::string name;

public:
    Instrument(const std::string &name);

    //std::string get_name() const;
    //void set_name(const std::string &name);

    //virtual double get_error() const;
    //virtual void set_error(double error);
    //virtual void set_error(std::vector<double> error);
    virtual void add_error(double error);
};
