#include "Instrument.h"
#include <stdexcept>

Instrument::Instrument(const std::string &name, double error)
    : name(name),
      error(error)
{
}

Instrument::Instrument()
{
}

std::string Instrument::get_name() const
{
    return name;
}

double Instrument::get_error() const
{
    return error;
}

void Instrument::set_name(const std::string &name)
{
    this->name = name;
}

void Instrument::set_error(double error)
{
    this->error = error;
}