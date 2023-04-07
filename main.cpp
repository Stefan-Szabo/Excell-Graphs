#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_set>

void read_config(int *nr_p, int *nr_d, std::string *filename);
std::vector<std::vector<std::string>> readCSV(const std::string& filename, int nr_people, int nr_days);
std::vector<std::vector<std::string>> modify_m(const std::vector<std::vector<std::string>>& data);
void writeCSV(const std::string& filename, const std::vector<std::vector<std::string>>& data);
std::vector<std::vector<std::string>> createSchedule(const std::vector<std::vector<std::string>>& inputSchedule, int nr_people, int nr_days);

int main(){
    int nr_people{0}, nr_days{0};
    std::string filename{};
    read_config(&nr_people, &nr_days, &filename);
    std::cout << "nr_people=" << nr_people << std::endl; //debuging for config file reading
    std::cout << "nr_days=" << nr_days << std::endl;
    std::cout << "nume fisier=" << filename << std::endl;
    std::vector<std::vector<std::string>> csvData = readCSV(filename, nr_people, nr_days);
    /*for (int i = 4; i <= 26; i++) {
        for (int j = 3; j <= 33; j++) {
            std::cout << csvData[i][j] << " ";
        }
        std::cout << std::endl;
    }*/
    std::vector<std::vector<std::string>> csvDataModified = createSchedule(csvData, nr_people, nr_days);
    for (int i = 4; i <= 26; i++) {
        for (int j = 3; j <= 33; j++) {
            std::cout << csvDataModified[i][j] << " ";
        }
        std::cout << std::endl;
    }
    //writeCSV("modified_data.csv", csvDataModified);
    return 0;
}

//reading funtion of config file, first value represents the number of people from the excel table, second value number of days of the month 
void read_config(int *nr_p, int *nr_d, std::string *filename){
    int p{0}, d{0};
    std::string file;
    std::ifstream in_file;
    in_file.open("configuration.txt");
    if (!in_file){
        std::cerr << "Problem open configuration file" << std::endl;
    }
    in_file >> p >> d >> file;
    *nr_p = p;
    *nr_d = d;
    *filename = file;

}

std::vector<std::vector<std::string>> readCSV(const std::string& filename, int nr_people, int nr_days) {
    std::ifstream inputFile(filename);
    if (!inputFile.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return {};
    }
    // Read the file line by line
    std::string line;
    std::vector<std::vector<std::string>> data;
    while (std::getline(inputFile, line)) {
        // Split the line into fields using a stringstream
        std::stringstream ss(line);
        std::vector<std::string> fields;
        std::string field;
        while (std::getline(ss, field, ',')) {
            fields.push_back(field);
        }
        data.push_back(fields);
    }
    inputFile.close();
    return data;
}

std::vector<std::vector<std::string>> modify_m(const std::vector<std::vector<std::string>>& data) {
    std::vector<std::vector<std::string>> modifiedData = data;
    for (int i = 4; i <= 26; i++) {
        for (int j = 3; j <= 33; j++) {
            modifiedData[i][j] += "_m";
        }
    }
    return modifiedData;
}

void writeCSV(const std::string &filename, const std::vector<std::vector<std::string>> &data) {
    std::ofstream outputFile(filename);
    if (!outputFile.is_open()) {
        std::cerr << "Failed to open file for writing: " << filename << std::endl;
        return;
    }
    // Write each row of data to the file
    for (const auto& row : data) {
        for (const auto& field : row) {
            outputFile << field << ",";
        }
        outputFile << "\n";
    }
    outputFile.close();
}

std::vector<std::vector<std::string>> createSchedule(const std::vector<std::vector<std::string>>& inputSchedule, int nr_people, int nr_days) {
    const int NUM_WORKING_DAYS = 22; // number of working days in a month (assuming 30-day month)
    const int NUM_PERSONS_DAY = 5; // minimum number of persons needed for day shift
    const int NUM_PERSONS_NIGHT = 2; // number of persons needed for night shift
    const int NUM_N_NIGHTS = 3; // minimum number of N (12 hour night shift) for each person
    const std::vector<std::string> SHIFTS = {"Z", "N", "1", "2"}; // available shifts
    const int NUM_DAYS_OFF_FOR_12H_SHIFT = 1; // number of days off needed after working a 12-hour shift
    const int HOURS_PER_SHIFT = 12; // length of a shift in hours

    // calculate the number of N (12 hour night shift) needed per person based on existing CO days
    int numNPerPerson = NUM_N_NIGHTS;
    std::vector<int> coDays;
    for (int i = 4; i < nr_people+4; ++i) {
        for (int j = 3; j < nr_days+3; ++j) {
            if (inputSchedule[i][j] == "CO")
                coDays.push_back(j);
        }
    }
    if (coDays.size() > 0) {
        int numWorkingDays = NUM_WORKING_DAYS - coDays.size();
        numNPerPerson = 3 * numWorkingDays / NUM_WORKING_DAYS;
        if (numNPerPerson < NUM_N_NIGHTS)
            numNPerPerson = NUM_N_NIGHTS;
    }

    // create a copy of the input schedule
    std::vector<std::vector<std::string>> schedule = inputSchedule;

    // create a set of workers who have already worked a 12-hour shift in the last 24 hours
    std::unordered_set<int> workersOff;

    // iterate over each person in the schedule
    for (int i = 0; i < schedule.size(); ++i) {
        int numN = 0; // number of N (12 hour night shift) already assigned to the person
        int numNNeeded = numNPerPerson; // number of N (12 hour night shift) needed by the person
        int numDaysOff = 0; // number of days off after working a 12-hour shift

        // iterate over each day in the schedule
        for (int j = 0; j < schedule[i].size(); ++j) {
            std::string &shift = schedule[i][j];

            // skip empty cells
            if (shift.empty())
                continue;

            // skip already assigned shifts
            if (shift != "1" && shift != "2" && shift != "N")
                continue;

            // check if worker is off due to 12-hour shift
            if (workersOff.count(i) > 0) {
                ++numDaysOff;
                if (numDaysOff >= NUM_DAYS_OFF_FOR_12H_SHIFT) {
                    workersOff.erase(i);
                    numDaysOff = 0;
                } else {
                    continue;
                }
            }

            // assign a day shift if needed
            if (shift == "1" || shift == "2") {
                if (numNNeeded > 0)
                    continue;

                int numPersons = 0;
                // count the number of persons already assigned to day shift
                for (int k = 0; k < schedule.size(); ++k) {
                    if (k == i)
                        continue;
                    if (schedule[k][j] == "1" || schedule[k][j] == "2")
                        ++numPersons;
                }

                // assign a day shift if enough persons are available
                if (numPersons < NUM_PERSONS_DAY) {
                    shift = "1";
                } else {
                    shift = "2";
                }
            }

            // assign a night shift if needed
            if (shift == "N") {
                if (numN >= numNNeeded) {
                    continue;
                }

                // count the number of persons already assigned to night shift
                int numPersons = 0;
                for (int k = 0; k < schedule.size(); ++k) {
                    if (k == i)
                        continue;
                    if (schedule[k][j] == "N")
                        ++numPersons;
                }

                // assign a night shift if enough persons are available
                if (numPersons < NUM_PERSONS_NIGHT) {
                    ++numN;
                    shift = "N";
                    if (numN == numNNeeded) {
                        workersOff.insert(i);
                        numDaysOff = 0;
                    }
                }
            }
        }
    }

    return schedule;
}




