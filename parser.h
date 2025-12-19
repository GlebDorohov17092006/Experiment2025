#pragma once

#include <string>
#include <vector>
#include <memory>
#include "Variable.h"

class Instrument;

//General parser
std::vector<Variable> parser(const std::string& filename_csv, const std::string& filename_json, std::vector<std::shared_ptr<Instrument>>* instruments = nullptr, std::shared_ptr<Instrument> noInstrument = nullptr);

