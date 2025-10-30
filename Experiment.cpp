#include "Experiment.h"
#include <stdexcept>

Experiment *Experiment::instance = nullptr;

Experiment::Experiment(std::vector<Variable> variables, std::vector<Variable> calculated_variables)
    : variables(variables),
      calculated_variables(calculated_variables)
{
}

Experiment *Experiment::get_instance(std::vector<Variable> variables, std::vector<Variable> calculated_variables)
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
    if (index >= variables.size())
    {
        throw std::out_of_range("The index goes beyond the bounds of the variables array");
    }
    return std::make_shared<Variable>(variables[index]);
}

std::shared_ptr<Variable> Experiment::get_calculated_variable(size_t index) const
{
    if (index >= calculated_variables.size())
    {
        throw std::out_of_range("The index goes beyond the bounds of the calculated_variables array");
    }
    return std::make_shared<Variable>(calculated_variables[index]);
}

size_t Experiment::get_variables_count() const
{
    return variables.size();
}

size_t Experiment::get_calculated_variables_count() const
{
    return calculated_variables.size();
}

void Experiment::add_variable(const Variable &variable)
{
    variables.push_back(variable);
}

void Experiment::add_calculated_variable(const Variable &variable)
{
    calculated_variables.push_back(variable);
}

void Experiment::remove_calculated_variable(size_t index)
{
    if (index >= calculated_variables.size())
    {
        throw std::out_of_range("The index goes beyond the bounds of the calculated_variables array");
    }
    calculated_variables.erase(calculated_variables.begin() + index);
}

void Experiment::remove_variable(size_t index)
{
    if (index >= variables.size())
    {
        throw std::out_of_range("The index goes beyond the bounds of the variables array");
    }
    variables.erase(variables.begin() + index);
}

void Experiment::set_variable(size_t index, const Variable &variable)
{
    if (index >= variables.size())
    {
        throw std::out_of_range("The index goes beyond the bounds of the variables array");
    }
    variables[index] = variable;
}

void Experiment::set_calculated_variable(size_t index, const Variable &variable)
{
    if (index >= calculated_variables.size())
    {
        throw std::out_of_range("The index goes beyond the bounds of the calculated_variables array");
    }
    calculated_variables[index] = variable;
}