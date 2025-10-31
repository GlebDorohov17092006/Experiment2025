#include "AbsoluteInstrument.h"

AbsoluteInstrument::AbsoluteInstrument(const std::string &name, double error)
    : Instrument(name), error(error)
{
}

double AbsoluteInstrument::get_error(size_t index, double value) const
{
    (void)index;
    (void)value;
    return error;
}

void AbsoluteInstrument::set_error(size_t index, double error)
{
    (void)index;
    this->error = error;
}