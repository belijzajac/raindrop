#include "FetchWeather.h"
#include "Conditions.h"

#include <iostream>

int main() {
    std::string url = "https://api.apixu.com/v1/forecast.json?key=4028be11da914310a1f133957182707&q=Vilnius&days=3";
    auto *fetchWeather = new FetchWeather(url);

    fetchWeather->showJSONdata();

    std::cout << "\nParsing data:\n";
    fetchWeather->parseJSON();

    delete fetchWeather;
    return 0;
}