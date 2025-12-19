#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <map>
#include <memory>
#include "json.hpp"
#include <qDebug>
#include "parser.h"
#include "Experiment.h"
#include "Instrument.h"
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
        return {};
    }

    if (!csv_file.is_open()) {
        std::cerr << "Open fault " << filename << std::endl;
        return {};
    }

    int num_line = 1;

    //Line-by-line parsing of the table
    while(std::getline(csv_file, header_of_table))
    {
        // Удаляем все кавычки из строки без каких-либо проверок
        header_of_table.erase(
            std::remove(header_of_table.begin(), header_of_table.end(), '\"'),
            header_of_table.end()
        );

        //Parsing curr line of the table
        std::stringstream row_of_table(header_of_table);
        std::string cell_of_table;
        int num_of_sell = 0;

        while(getline(row_of_table, cell_of_table, ','))
        {
            // Убираем пробелы и перевод строки по краям
            while (!cell_of_table.empty() && (cell_of_table.back() == ' ' || cell_of_table.back() == '\r' || cell_of_table.back() == '\t'))
                cell_of_table.pop_back();
            size_t firstNotSpace = cell_of_table.find_first_not_of(" \t\r");
            if (firstNotSpace != std::string::npos)
                cell_of_table = cell_of_table.substr(firstNotSpace);

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
                double value = 0.0;
                try
                {
                    if (!cell_of_table.empty())
                    {
                        value = std::stod(cell_of_table);
                    }
                }
                catch (const std::exception&)
                {
                }

                if (num_of_sell < static_cast<int>(variables.size()))
                {
                    variables[num_of_sell].add_measurement(value);
                }
            }

            num_of_sell += 1;
        }

        num_line += 1;
    }

    csv_file.close();

    return variables;
}

void parser_json(std::vector<Variable> &variables, const std::string& filename, std::vector<std::shared_ptr<Instrument>>* instruments, std::shared_ptr<Instrument> noInstrument)
{
    //Reading json file
    std::string full_path = filename;
    std::ifstream json_file(full_path);

    // If not found, try relative path from build directory
    if (!json_file.is_open()) {
        // Если файл не открыт, устанавливаем базовый инструмент для всех переменных
        if (noInstrument) {
            for(Variable& curr_variable : variables) {
                if (curr_variable.get_instrument() == nullptr) {
                    curr_variable.add_instrument(noInstrument.get());
                }
            }
        }
        return;
    }

    if (!json_file.is_open()) {
        std::cerr << "Open fault " << filename << std::endl;
        // Если файл не открыт, устанавливаем базовый инструмент для всех переменных
        if (noInstrument) {
            for(Variable& curr_variable : variables) {
                if (curr_variable.get_instrument() == nullptr) {
                    curr_variable.add_instrument(noInstrument.get());
                }
            }
        }
        return;
    }

    json instruments_data;
    try {
        instruments_data = json::parse(json_file);
    } catch (const json::parse_error& e) {
        std::cerr << "JSON parse error: " << e.what() << std::endl;
        json_file.close();
        // При ошибке парсинга устанавливаем базовый инструмент для всех переменных
        if (noInstrument) {
            for(Variable& curr_variable : variables) {
                if (curr_variable.get_instrument() == nullptr) {
                    curr_variable.add_instrument(noInstrument.get());
                }
            }
        }
        return;
    }

    // Map для хранения уникальных инструментов по имени
    std::map<std::string, std::shared_ptr<Instrument>> instruments_map;

    //For each column of table setting name of instrument and error
    for(Variable& curr_variable : variables)
    {
        try {
            std::string variable_name = curr_variable.get_name_tables();
            
            // Проверяем, существует ли переменная в JSON
            if (!instruments_data["Variables"].contains(variable_name)) {
                // Если переменной нет в JSON, устанавливаем базовый инструмент
                if (noInstrument && curr_variable.get_instrument() == nullptr) {
                    curr_variable.add_instrument(noInstrument.get());
                }
                continue;
            }
            
            std::string name_instrument = instruments_data["Variables"][variable_name].get<std::string>();
            
            // Проверяем, существует ли инструмент в JSON
            if (!instruments_data["Instruments"].contains(name_instrument)) {
                // Если инструмента нет в JSON, устанавливаем базовый инструмент
                if (noInstrument && curr_variable.get_instrument() == nullptr) {
                    curr_variable.add_instrument(noInstrument.get());
                }
                continue;
            }
            
            std::string type_of_error = instruments_data["Instruments"][name_instrument]["type"].get<std::string>();
            double value_of_error = instruments_data["Instruments"][name_instrument]["error"].get<double>();

            // Проверяем, есть ли уже такой инструмент в map
            std::shared_ptr<Instrument> instrument;
            if (instruments_map.find(name_instrument) != instruments_map.end()) {
                // Используем существующий инструмент
                instrument = instruments_map[name_instrument];
            } else {
                // Создаем новый инструмент
                if(type_of_error == "Absolute")
                {
                    instrument = std::make_shared<AbsoluteInstrument>(name_instrument, value_of_error);
                }
                else if(type_of_error == "Relative")
                {
                    instrument = std::make_shared<RelativeInstrument>(name_instrument, value_of_error);
                }
                
                // Добавляем в map
                if (instrument) {
                    instruments_map[name_instrument] = instrument;
                }
            }

            // Добавляем инструмент к переменной
            if (instrument) {
                curr_variable.add_instrument(instrument.get());
            }
        } catch (const std::exception& e) {
            std::cerr << "Error processing variable: " << e.what() << std::endl;
            // При ошибке устанавливаем базовый инструмент
            if (noInstrument && curr_variable.get_instrument() == nullptr) {
                curr_variable.add_instrument(noInstrument.get());
            }
            continue;
        }
    }

    // Если передан указатель на вектор инструментов, заполняем его
    if (instruments != nullptr) {
        instruments->clear();
        // Базовый инструмент будет добавлен в MainWindow, здесь добавляем только инструменты из JSON
        for (const auto& pair : instruments_map) {
            instruments->push_back(pair.second);
        }
    }

    json_file.close();
}

std::vector<Variable> parser(const std::string& filename_csv, const std::string& filename_json, std::vector<std::shared_ptr<Instrument>>* instruments, std::shared_ptr<Instrument> noInstrument)
{
    std::vector<Variable> variables =  parser_csv(filename_csv);
    parser_json(variables, filename_json, instruments, noInstrument);

    return variables;

}



