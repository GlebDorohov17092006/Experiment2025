#pragma once

#include <string>
#include <vector>
#include "variable.h"

void parser_json(std::vector<Variable> &variables);

std::vector<Variable> parser_csv(const std::string& filename);