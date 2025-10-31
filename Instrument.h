#pragma once

#include <string>

class Instrument
{
protected:
    std::string name;

public:
    Instrument(const std::string &name = "");
    virtual ~Instrument() = default;
    std::string get_name() const;
    virtual double get_error(size_t index = 0, double value = 0.0) const;
    void set_name(const std::string &name);
    virtual void set_error(size_t index = 0, double error = 0.0);
};
