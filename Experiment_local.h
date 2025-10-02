#pragma once

#include "Variable.h"
#include <memory>

class Experiment
{
private:
    std::vector<Variable> variables;
    std::vector<Variable> calculated_variables;

public:
    Experiment(std::vector<Variable> variables, std::vector<Variable> calculated_variables);
    std::shared_ptr<Variable> get_calculated_variable(size_t index) const;
    std::shared_ptr<Variable> get_variable(size_t index) const;
    size_t get_variables_count() const;
    size_t get_calculated_variables_count() const;
    void add_variable(const Variable &variable);
    void add_calculated_variable(const Variable &variable);
    void remove_calculated_variable(size_t index);
    void remove_variable(size_t index);
};