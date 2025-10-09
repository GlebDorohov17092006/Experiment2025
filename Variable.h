#pragma once

#include <vector>
#include <string>

class Variable
{
private:
    std::vector<double> measurements;
    std::string name_tables;
    std::string name_calculated;
    std::vector<double> errors;
    std::string instrument;

public:
    Variable(std::vector<double> measurements, const std::string &name_tables,
             const std::string &name_calculated, std::vector<double> errors,
             const std::string &instrument);
    double get_measurement(size_t index) const;
    std::string get_name_tables() const;
    std::string get_name_calculated() const;
    std::string get_instrument() const;
    double get_error(size_t index) const;
    size_t get_measurements_count() const;
    void set_measurement(size_t index, double value);
    void set_name_tables(const std::string &name);
    void set_name_calculated(const std::string &name_calculated);
    void set_instrument(const std::string &instrument);
    void set_error(size_t index, double error);
    void add_measurement(double measurement);
    void add_error(double error);
};