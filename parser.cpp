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
    json instruments_data = json::parse(json_file);

    //For each column of table setting name of instrument and error
    for(Variable& curr_variable : variables)
    { 
        //Pulling out data of instrument
        std::string variable_name = curr_variable.get_name_tables();
        std::string name_instrument = instruments_data["Variables"][variable_name];
        std::string type_of_error = instruments_data["Instruments"][name_instrument]["type"];
        std::string value_of_error_string = instruments_data["Instruments"][name_instrument]["error"];
        double value_of_error = atof(value_of_error_string.c_str());
        
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