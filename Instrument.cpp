#include "Instrument.h"

Instrument::Instrument(const std::string &name)
    : name(name)
{
}
/*
std::string Instrument::get_name() const
{
    return name;
}

void Instrument::set_name(const std::string &name)
{
    this->name = name;
}
*/

void Instrument::add_error(double error)
{
}
