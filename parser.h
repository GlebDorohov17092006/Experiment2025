#pragma once

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include "variable.h"

std::vector<Variable> parser_csv(const std::string& filename)
{
    std::vector<Variable> variables;
    std::string header_of_table;
    std::ifstream csv_file(filename);
    int num_line = 1;

    if(csv_file.is_open())
    {
        std::cerr << "File is not found!" << std::endl;
        //Выкинуть ошибку в виде всплывающего окна

        return variables;
    }

    //Построчный парсинг таблицы
    while(std::getline(csv_file, header_of_table))
    {
        //Парсинг строк таблицы
        std::stringstream row_of_table(header_of_table);
        std::string cell_of_table;
        int num_of_sell = 0;

        while(getline(row_of_table, cell_of_table, ','))
        {
            //Для атрибутов таблицы создаем структуры variable
            if(num_line == 1)
            {
                //Добавление переменных в вектор
                Variable param_table;
                param_table.set_name_tables(cell_of_table);

                variables[num_of_sell] = param_table;
            } 
            else
            {
                //Добавление экспериментальных данных
                variables[num_of_sell].set_measurement(num_of_sell, atof(cell_of_table.c_str()));
            }

            num_of_sell += 1;
        }
        
        num_line += 1;
    }

    csv_file.close();
    return variables;
}







#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

std::vector<std::vector<std::string>> parseCSV(const std::string& filename) {
    std::vector<std::vector<std::string>> data;
    std::ifstream file(filename);
    std::string line;
    
    if (!file.is_open()) {
        std::cerr << "Ошибка открытия файла!" << std::endl;
        return data;
    }
    
    while (std::getline(file, line)) {
        std::vector<std::string> row;
        std::stringstream ss(line);
        std::string cell;
        
        while (std::getline(ss, cell, ',')) {
            // Удаляем кавычки если есть
            if (!cell.empty() && cell.front() == '"' && cell.back() == '"') {
                cell = cell.substr(1, cell.size() - 2);
            }
            row.push_back(cell);
        }
        data.push_back(row);
    }
    
    file.close();
    return data;
}