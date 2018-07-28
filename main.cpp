#include "FetchWeather.h"

#include <iostream>

int main() {
    std::string url = "https://api.apixu.com/v1/forecast.json?key=4028be11da914310a1f133957182707&q=Vilnius";
    auto *fetchWeather = new FetchWeather(url);

    fetchWeather->showJSONdata();

    delete fetchWeather;
    return 0;
}