#pragma once

#include <string>
#include <vector>
#include "Variable.h"

//General parser
std::vector<Variable> parser(const std::string& filename_csv, const std::string& filename_json);

