#pragma once

#include "Instrument.h"

class AbsoluteInstrument : public Instrument
{
private:
    double error;

public:
    AbsoluteInstrument(const std::string &name = "", double error = 0.0);
    double get_error(size_t index = 0, double value = 0.0) const override;
    void set_error(size_t index, double error) override;
};