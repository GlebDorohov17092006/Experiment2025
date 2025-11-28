#include "Variable.h"
#include <stdexcept>
#include <iostream>

Variable::Variable(std::vector<double> measurements, const std::string &name_tables,
                   const std::string &name_calculated, Instrument *instrument)
    : measurements(measurements),
    name_tables(name_tables),
    name_calculated(name_calculated),
    instrument(instrument)
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

double Variable::get_measurement(size_t index) const
{
    if (index >= measurements.size())
    {
        return 0.0;
    }
    return measurements[index];
}

std::vector<double>& Variable::get_measurements()
{
    return measurements;
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
        return;
    }
    measurements[index] = value;
}

void Variable::add_measurement(double measurement)
{
    measurements.push_back(measurement);
}

std::string Variable::get_name_instrument() const
{
    if (instrument == nullptr)
    {
        return "(нет инструмента)";
    }
    return instrument->get_name();
}

double Variable::get_error_instrument(size_t index, double value) const
{
    if (instrument == nullptr)
    {
        return 0.0;
    }

    return instrument->get_error(index, value);
}

void Variable::set_name_instrument(const std::string &name)
{
    if (instrument == nullptr)
    {
        return;
    }
    instrument->set_name(name);
}

void Variable::set_error_instrument(size_t index, double error)
{
    if (instrument == nullptr)
    {
        return;
    }
    instrument->set_error(index, error);
}

void Variable::add_instrument(Instrument *instrument)
{
    this->instrument = instrument;
}

void Variable::remove_measurement(size_t index)
{
    if (index >= measurements.size())
    {
        return;
    }
    measurements.erase(measurements.begin() + static_cast<std::ptrdiff_t>(index));
}
