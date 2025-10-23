#pragma once

#include <vector>
#include <string>
#include "Instrument.h"
#include "AbsoluteInstrument.h"
#include "RelativeInstrument.h"

class Variable
{
private:
    std::vector<double> measurements;
    std::string name_tables;
    std::string name_calculated;
    std::vector<Instrument *> instruments;

public:
    Variable(std::vector<double> measurements, const std::string &name_tables,
             const std::string &name_calculated, const Instrument &instrument);
    Variable();
    double get_measurement(size_t index) const;
    std::string get_name_tables() const;
    std::string get_name_calculated() const;
    Instrument get_instrument(size_t index) const;
    size_t get_measurements_count() const;
    void set_measurement(size_t index, double value);
    void set_name_tables(const std::string &name);
    void set_name_calculated(const std::string &name_calculated);
    void set_name_instrument(size_t index, const std::string &name);
    void set_error_instrument(size_t index, double error);
    void add_measurement(double measurement);
    void add_instrument(Instrument *Instrument);
};