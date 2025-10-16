#include <fstream>
#include <iostream>
#include <sstream>
#include "json.hpp"
#include "parser.h"

using json = nlohmann::json;

void parser_json(std::vector<Variable> &variables, const std::string& filename)
{
    //Reading json file
    std::ifstream json_file(filename);
    json tools_errors = json::parse(json_file);

    //For each column of table setting name of instrument and error
    for(Variable& curr_variable : variables)
    {   
        std::string curr_variable_table_name = curr_variable.get_name_tables();
        curr_variable.set_name_instrument(tools_errors[curr_variable_table_name]["name_of_instrument"]);
        curr_variable.set_error_instrument(tools_errors[curr_variable_table_name]["value_of_error"])
    }

    json_file.close();
}

std::vector<Variable> parser_csv(const std::string& filename)
{
    std::vector<Variable> variables;
    std::string header_of_table;
    std::ifstream csv_file(filename);
    int num_line = 1;

    if(csv_file.is_open())
    {
        std::cerr << "File is not found!" << std::endl;
        //Throw error of open file

        return variables;
    }

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

    //Getting instruments and errors for each variables
    parser_json(variables);

    return variables;
}