#include "Instrument.h"

Instrument::Instrument(const std::string &name)
    : name(name)
{
}

std::string Instrument::get_name() const
{
    return name;
}

void Instrument::set_name(const std::string &name)
{
    this->name = name;
}

double Instrument::get_error(size_t index, double value) const
{
    (void)index;
    (void)value;
    return 0.0;
}

void Instrument::set_error(size_t index, double error)
{
    (void)index;
    (void)error;
}