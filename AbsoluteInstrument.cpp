#include "AbsoluteInstrument.h"

AbsoluteInstrument::AbsoluteInstrument()
{
}

AbsoluteInstrument::AbsoluteInstrument(const std::string &name, double error)
    : Instrument(name), error(error)
{
}

double AbsoluteInstrument::get_error() const
{
    return error;
}

void AbsoluteInstrument::set_error(double error)
{
    this->error = error;
}