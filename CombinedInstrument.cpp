#include "CombinedInstrument.h"
#include <stdexcept>

CombinedInstrument::CombinedInstrument(const std::string &name)
    : Instrument(name)
{
}

double CombinedInstrument::get_error(size_t index, double value) const
{
    (void)value;
    if (index >= errors.size())
    {
        throw std::out_of_range("The index goes beyond the bounds of the index array");
    }
    return errors[index];
}

void CombinedInstrument::set_error(size_t index, double error)
{
    if (index >= errors.size())
    {
        errors.resize(index + 1, 0.0);
    }
    errors[index] = error;
}

void CombinedInstrument::add_error(double error)
{
    errors.push_back(error);
}
