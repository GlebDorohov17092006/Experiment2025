#include "AbsoluteInstrument.h"

AbsoluteInstrument::AbsoluteInstrument(const std::string &name, double error)
    : Instrument(name), error(error)
{
}
/*
double AbsoluteInstrument::get_error() const
{
    return this->error;
}

void AbsoluteInstrument::set_error(double error)
{
    this->error = error;
}
*/
void AbsoluteInstrument::add_error(double error)
{
    this->error = error;
}
