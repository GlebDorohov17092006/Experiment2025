#include <fstream>
#include <iostream>
#include <sstream>
#include "json.hpp"
#include "parser.h"
#include "AbsoluteInstrument.h"
#include "RelativeInstrument.h"
using json = nlohmann::json;

std::vector<Variable> parser_csv(const std::string& filename)
{
    std::vector<Variable> variables;
    std::string header_of_table;

    // Try to find the file in the project root directory
    std::string full_path = filename;
    std::ifstream csv_file(full_path);

    // If not found, try relative path from build directory
    if (!csv_file.is_open()) {
        full_path = "../../" + filename;
        csv_file.open(full_path);
    }

    if (!csv_file.is_open()) {
        std::cerr << "Open fault " << filename << std::endl;
        exit(-3);
    }

    int num_line = 1;

    //Line-by-line parsing of the table
    while(std::getline(csv_file, header_of_table))
    {
        //Parsing curr line of the table
        std::stringstream row_of_table(header_of_table);
        std::string cell_of_table;
        int num_of_sell = 0;

        while(getline(row_of_table, cell_of_table, ','))
        {
            //Creating object of class variable for curr column
            if(num_line == 1)
            {
                //Adding object of variable in vector
                Variable column_of_table;
                column_of_table.set_name_tables(cell_of_table);

                variables.push_back(column_of_table);
            }
            else
            {
                //Adding experimental data
                variables[num_of_sell].add_measurement(atof(cell_of_table.c_str()));
            }

            num_of_sell += 1;
        }

        num_line += 1;
    }

    csv_file.close();

    return variables;
}

void parser_json(std::vector<Variable> &variables, const std::string& filename)
{
    //Reading json file
    std::string full_path = filename;
    std::ifstream json_file(full_path);

    // If not found, try relative path from build directory
    if (!json_file.is_open()) {
        full_path = "../../" + filename;
        json_file.open(full_path);
    }

    if (!json_file.is_open()) {
        std::cerr << "Open fault " << filename << std::endl;
        exit(-3);
    }

    json instruments_data = json::parse(json_file);

    //For each column of table setting name of instrument and error
    for(Variable& curr_variable : variables)
    {
        //Pulling out data of instrument
        std::string variable_name = curr_variable.get_name_tables();
        std::string name_instrument = instruments_data["Variables"][variable_name];
        std::string type_of_error = instruments_data["Instruments"][name_instrument]["type"];
        double value_of_error = instruments_data["Instruments"][name_instrument]["error"];

        //Creating instrument and adding it to each variable
        if(type_of_error == "Absolute")
        {
            AbsoluteInstrument instrument(name_instrument, value_of_error);
            curr_variable.add_instrument(&instrument);
        }
        else
        {
            RelativeInstrument instrument(name_instrument, value_of_error);
            curr_variable.add_instrument(&instrument);
        }
    }

    json_file.close();
}

std::vector<Variable> parser(const std::string& filename_csv, const std::string& filename_json)
{
    std::vector<Variable> variables =  parser_csv(filename_csv);
    parser_json(variables, filename_json);

    return variables;

}



