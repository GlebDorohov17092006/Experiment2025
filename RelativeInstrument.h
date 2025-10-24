#pragma once

#include "Instrument.h"

class RelativeInstrument : public Instrument
{
private:
    double error;

public:
    RelativeInstrument(const std::string &name = "", double error = 0.0);
    //double get_error() const override;
    //void set_error(double error) override;
    void add_error(double error) override;
};
