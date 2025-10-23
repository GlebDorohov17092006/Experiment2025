#pragma once

#include "Instrument.h"

class AbsoluteInstrument : public Instrument
{
private:
    double error;

public:
    AbsoluteInstrument(const std::string &name = "", double error = 0.0);
    double get_error() const override;
    void set_error(double error) override;
};