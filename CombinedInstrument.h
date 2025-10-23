#pragma once

#include <vector>
#include "Instrument.h"

class CombinedInstrument : public Instrument
{
private:
    std::vector<double> errors;

public:
    double get_error(size_t index, double value) const override;
    void set_error(size_t index, double error) override;
    void add_error(double error);
};