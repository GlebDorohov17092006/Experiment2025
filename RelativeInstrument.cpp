#include "RelativeInstrument.h"

RelativeInstrument::RelativeInstrument(const std::string &name, double error)
    : Instrument(name), error(error)
{
}

double RelativeInstrument::get_error(size_t index, double value) const
{
    (void)index;
    return error * value;
}

void RelativeInstrument::set_error(size_t index, double error)
{
    (void)index;
    this->error = error;
}
