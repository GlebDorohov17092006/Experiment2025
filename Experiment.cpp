#include "Experiment.h"
#include <stdexcept>
#include <memory>

Experiment *Experiment::instance = nullptr;

Experiment::Experiment(std::vector<Variable> variables,
                       std::vector<Variable> calculated_variables)
    : variables(variables),
      calculated_variables(calculated_variables)
{
}

Experiment *Experiment::get_instance(std::vector<Variable> variables,
                                     std::vector<Variable> calculated_variables)
{
    if (instance == nullptr)
    {
        instance = new Experiment(variables, calculated_variables);
    }
    else if (!variables.empty() || !calculated_variables.empty())
    {
        // Обновляем переменные, если они переданы и instance уже существует
        instance->variables = variables;
        instance->calculated_variables = calculated_variables;
    }
    return instance;
}

void Experiment::destroy_instance()
{
    if(instance != nullptr)
        delete instance;
    instance = nullptr;
}

Experiment::~Experiment()
{
}

Variable& Experiment::get_variable(size_t index)
{
    return variables.at(index);
}

Variable& Experiment::get_calculated_variable(size_t index)
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

void Experiment::add_variable(const Variable& variable)
{
    variables.push_back(variable);
}

void Experiment::add_calculated_variable(const Variable& variable)
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

void Experiment::set_variable(size_t index, const Variable& variable)
{
    variables[index] = variable;
}

void Experiment::set_calculated_variable(size_t index, const Variable& variable)
{
    calculated_variables[index] = variable;
}
