#pragma once

#include <vector>
#include <string>

class Variable
{

private:
    std::vector<double> measurements;
    std::string name_tables;
    std::string name_calc;
    std::vector<double> error;
    std::string instrument;

public:

    std::vector<double> get_measurements();
    std::string get_name_tables();
    std::string get_name_calc();
    std::string get_instrument();
    std::vector<double> get_error();
    int get_measurements_count();


    void set_measurement(int index, double value);
    void set_name_tables(std::string new_name);
    void set_name_calc(std::string new_name_calc);
    void set_instrument(std::string new_instrument);
    void set_measurement_count(int new_count);
    void set_error(int index, double err);

};