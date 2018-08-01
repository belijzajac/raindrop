#include "Config.h"
#include "Exception.h"
#include "FetchWeather.h"
#include "Conditions.h"

#include <iostream>

int main() {
    auto *config = new Config;
    bool is_ok_to_proceed = true;

    // Just to make sure the config file exists and is correctly configured
    try{
        config->init();
    }catch(const Exception_File &exception){
        std::cout << "Caught an exception: " + std::string(exception.what()) << std::endl;
        is_ok_to_proceed = false;
    }catch(...){
        std::cout << "Caught an exception of an undetermined type\n";
        is_ok_to_proceed = false;
    }

    // Proceed to get weather report
    if(is_ok_to_proceed){
        auto *fetchWeather = new FetchWeather(config->get_url());
            fetchWeather->parseJSON();
            fetchWeather->displayWeather();
        delete fetchWeather;
    }

    delete config;
    return 0;
}