#pragma once

#include <vector>
#include <string>
#include "Instrument.h"

class Variable
{
private:
    std::vector<double> measurements;
    std::string name_tables;
    std::string name_calculated;
    Instrument *instrument;

public:
    Variable(std::vector<double> measurements = {}, const std::string &name_tables = "",
             const std::string &name_calculated = "", Instrument *instrument = nullptr);
    double get_measurement(size_t index) const;
    std::string get_name_tables() const;
    std::string get_name_calculated() const;
    double get_error_instrument(size_t index = 0, double value = 0.0) const;
    std::string get_name_instrument() const;
    size_t get_measurements_count() const;
    void set_measurement(size_t index, double value);
    void set_name_tables(const std::string &name);
    void set_name_calculated(const std::string &name_calculated);
    void set_name_instrument(const std::string &name);
    void set_error_instrument(size_t index = 0, double error = 0.0);
    void add_instrument(Instrument *instrument);
    void add_measurement(double measurement);
};
