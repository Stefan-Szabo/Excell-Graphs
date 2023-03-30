#include <stdexcept>
#include <iostream>
#include "libxl.h"
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <windows.h>


using namespace libxl;
void read_config(int *nr_p, int *nr_d);
std::vector<std::string> ReadExcelValues(const std::string& filename, int rowCount, int colCount);

int main(){
    int nr_people{0}, nr_days{0};
    read_config(&nr_people, &nr_days);
    std::cout << "nr_people=" << nr_people << std::endl; //debuging for config file reading
    std::cout << "nr_days=" << nr_days << std::endl;
    std::vector<std::string> vec = ReadExcelValues("test.xls", nr_people, nr_days);
    return 0;

}

//reading funtion of config file, first value represents the number of people from the excel table, second value number of days of the month 
void read_config(int *nr_p, int *nr_d){ 
    int p{0}, d{0};
    std::ifstream in_file;
    in_file.open("configuration.txt");
    if (!in_file){
        std::cerr << "Problem open configuration file" << std::endl;
    }
    in_file >> p >> d;
    *nr_p = p;
    *nr_d = d;
}

std::vector<std::string> ReadExcelValues(const std::string& filename, int rowCount, int colCount)
{
    std::vector<std::string> values;

    Book* book = xlCreateBook();
    if (!book)
    {
        throw std::runtime_error("Failed to create book");
    }

    int wfilenameLength = MultiByteToWideChar(CP_UTF8, 0, filename.c_str(), -1, NULL, 0);
    if (wfilenameLength == 0)
    {
        book->release();
        throw std::runtime_error("Failed to convert filename to wide string");
    }

    std::wstring wfilename(wfilenameLength, 0);
    if (MultiByteToWideChar(CP_UTF8, 0, filename.c_str(), -1, &wfilename[0], wfilenameLength) == 0)
    {
        book->release();
        throw std::runtime_error("Failed to convert filename to wide string");
    }

    if (!book->load(wfilename.c_str()))
    {
        book->release();
        throw std::runtime_error("Failed to load book");
    }

    Sheet* sheet = book->getSheet(0); // assuming data is in first sheet
    if (!sheet)
    {
        book->release();
        throw std::runtime_error("Failed to get sheet");
    }


    for (int i = 4; i < rowCount+4; ++i)
    {
        for (int j = 3; j < colCount+3; ++j)
        {
            const wchar_t* wvalue = sheet->readStr(i, j);
            if (wvalue)
            {
                std::size_t length = std::wcslen(wvalue);
                std::vector<char> buffer(length + 1);
                std::wcstombs(buffer.data(), wvalue, buffer.size());
                values.emplace_back(buffer.data());
            }
            else
            {
                values.emplace_back(" ");
            }
        }
    }

    book->release();

    return values;
}

