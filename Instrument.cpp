#include "Instrument.h"
#include <stdexcept>

Instrument::Instrument(const std::string &name, double error)
    : name(name),
      error(error)
{
}

std::string Instrument::get_name()
{
    return name;
}

double Instrument::get_error()
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