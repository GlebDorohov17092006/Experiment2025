#include <fstream>
#include "saver.h"
#include "json.hpp"

using json = nlohmann::json;

void saver_json(const std::string& filename)
{
    //Reading current data of json file
    std::ifstream json_curr_data(filename);
    json curr_data = json::parse(json_curr_data);

    for()
}