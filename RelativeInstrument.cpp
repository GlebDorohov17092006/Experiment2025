#include "RelativeInstrument.h"

RelativeInstrument::RelativeInstrument(const std::string &name, double error)
    : Instrument(name), error(error)
{
}
/*
double RelativeInstrument::get_error() const
{
    return this->error;
}

void RelativeInstrument::set_error(double error)
{
    this->error = error;
}
*/

void RelativeInstrument::add_error(double error)
{
    this->error = error;
}
