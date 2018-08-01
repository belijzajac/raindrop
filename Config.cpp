#include "Config.h"
#include "Exception.h"

#include <fstream>
#include <iostream>
#include <vector>

Config::Config() : url("https://api.apixu.com/v1/forecast.json?") {}

void check_validity_of_data(const std::string &str, const std::string &what){
    std::string::size_type pos = str.find(what);
    if(pos == std::string::npos)
        throw Exception_File("Improperly configured config.txt file: in " + what);
}

void Config::init() {
    // Open the config file
    std::ifstream config_file("config.txt");
    if(!config_file.is_open())
        throw Exception_File("Cannot open file config.txt");
    else{
        std::vector<std::string> input;
        // Read file line by line
        for(std::string line; getline(config_file, line); )
            input.push_back(line);

        // Check the file
        check_validity_of_data(input[0], "Key=");
        check_validity_of_data(input[1], "City=");
        check_validity_of_data(input[2], "Days=");


        // Parse output
        this->m_key = input[0].substr(4, input[0].length() - 4);  // API key
        this->m_city = input[1].substr(5, input[1].length() - 5); // City
        this->m_days = input[2].substr(5, input[2].length() - 5); // Days

        if(stoi(this->m_days) > 10 || stoi(this->m_days) < 0)
            throw Exception_File("Improperly configured config.txt file");

        config_file.close();
    }
    this->url += "key=" + this->m_key + "&q=" + this->m_city + "&days=" + this->m_days;
}

const std::string Config::get_url() const {
    return this->url;
}
