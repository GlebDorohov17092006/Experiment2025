#include "Variable.h"
#include <stdexcept>

Variable::Variable(std::vector<double> measurements, const std::string &name_tables,
                   const std::string &name_calculated, const Instrument &instrument)
    : measurements(measurements),
      name_tables(name_tables),
      name_calculated(name_calculated),
      instrument(instrument)
{
}

Variable::Variable()
{
}

std::string Variable::get_name_tables() const
{
    return name_tables;
}

std::string Variable::get_name_calculated() const
{
    return name_calculated;
}

Instrument Variable::get_instrument() const
{
    return instrument;
}

double Variable::get_measurement(size_t index) const
{
    if (index >= measurements.size())
    {
        throw std::out_of_range("The index goes beyond the bounds of the measurements array");
    }
    return measurements[index];
}

size_t Variable::get_measurements_count() const
{
    return measurements.size();
}

void Variable::set_name_tables(const std::string &name_tables)
{
    this->name_tables = name_tables;
}

void Variable::set_name_calculated(const std::string &name_calculated)
{
    this->name_calculated = name_calculated;
}

void Variable::set_measurement(size_t index, double value)
{
    if (index >= measurements.size())
    {
        throw std::out_of_range("The index goes beyond the bounds of the measurements array");
    }
    measurements[index] = value;
}

void Variable::add_measurement(double measurement)
{
    measurements.push_back(measurement);
}