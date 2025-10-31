#pragma once

#include "Instrument.h"

class RelativeInstrument : public Instrument
{
private:
    double error;

public:
    RelativeInstrument(const std::string &name = "", double error = 0.0);
    double get_error(size_t index, double value) const override;
    void set_error(size_t index, double error) override;
};
