#include "Experiment.h"
#include <stdexcept>

Experiment *Experiment::instance = nullptr;

Experiment::Experiment(std::vector<std::shared_ptr<Variable>> variables,
                       std::vector<std::shared_ptr<Variable>> calculated_variables)
    : variables(variables),
      calculated_variables(calculated_variables)
{
}

Experiment *Experiment::get_instance(std::vector<std::shared_ptr<Variable>> variables,
                                     std::vector<std::shared_ptr<Variable>> calculated_variables)
{
    if (instance == nullptr)
    {
        instance = new Experiment(variables, calculated_variables);
    }
    return instance;
}

void Experiment::destroy_instance()
{
    delete instance;
    instance = nullptr;
}

Experiment::~Experiment()
{
}

std::shared_ptr<Variable> Experiment::get_variable(size_t index) const
{
    return variables.at(index);
}

std::shared_ptr<Variable> Experiment::get_calculated_variable(size_t index) const
{
    return calculated_variables.at(index);
}

size_t Experiment::get_variables_count() const
{
    return variables.size();
}

size_t Experiment::get_calculated_variables_count() const
{
    return calculated_variables.size();
}

void Experiment::add_variable(const std::shared_ptr<Variable>& variable)
{
    variables.push_back(variable);
}

void Experiment::add_calculated_variable(const std::shared_ptr<Variable>& variable)
{
    calculated_variables.push_back(variable);
}

void Experiment::remove_calculated_variable(size_t index)
{
    calculated_variables.erase(calculated_variables.begin() + index);
}

void Experiment::remove_variable(size_t index)
{
    variables.erase(variables.begin() + index);
}

void Experiment::set_variable(size_t index, const std::shared_ptr<Variable>& variable)
{
    variables[index] = variable;
}

void Experiment::set_calculated_variable(size_t index, const std::shared_ptr<Variable>& variable)
{
    calculated_variables[index] = variable;
}
