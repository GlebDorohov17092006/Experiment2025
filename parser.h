#pragma once

#include <string>
#include <vector>
#include "variable.h"

//Parser for json file
void parser_json(std::vector<Variable> &variables, const std::string& filename);

//Parser for csv file
std::vector<Variable> parser_csv(const std::string& filename);

//General parser
std::vector<Variable> parser(const std::string& filename_csv, const std::string& filename_json);

