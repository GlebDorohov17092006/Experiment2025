#pragma once

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include "variable.h"

std::vector<Variable> parser_csv(const std::string& filename);