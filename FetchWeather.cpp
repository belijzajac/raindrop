#include "FetchWeather.h"
#include "Conditions.h"

#include <curl/curl.h>
#include <curl/easy.h>

#include <iostream>
#include <cstring>
#include <cmath>
#include <iomanip>

FetchWeather::FetchWeather(const std::string &url){
    CURL *curl;
    CURLcode res;
    struct curl_slist *headers = nullptr;

    // Where the writeMemoryCallback will write data to
    struct MemoryStruct chunk{};

    // Set headers
    headers = curl_slist_append(headers, "Accept: application/json");
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, "charsets: utf-8");

    // Init curl
    curl = curl_easy_init();
    if(curl){
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPGET, 1);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeMemoryCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&chunk);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");

        // Perform the request; res will get the return code
        res = curl_easy_perform(curl);

        // Check if everything went right
        if(CURLE_OK == res){
            char *ct;
            res = curl_easy_getinfo(curl, CURLINFO_CONTENT_TYPE, &ct);
            if(CURLE_OK == res && ct)
                json_data = chunk.memory;
        }else{
            std::cout << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        }
    }

    // Cleanup curl
    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);
    curl_global_cleanup();
    free(chunk.memory);
}

FetchWeather::~FetchWeather() {
    delete current_data;
    delete [] forecast_data;
}

size_t FetchWeather::writeMemoryCallback(void *contents, size_t size, size_t nmemb, void *buffer_in) {
    size_t realSize = size * nmemb;
    auto *mem = (struct MemoryStruct*) buffer_in;

    mem->memory = static_cast<char *>(realloc(mem->memory, mem->size + realSize + 1));
    if(mem->memory == nullptr){
        std::cout << "Not enough memory, realloc returned nullptr" << std::endl;
        return 0;
    }

    // Append acquired data from API with new one
    memcpy(&(mem->memory[mem->size]), contents, realSize);
    mem->size += realSize;
    mem->memory[mem->size] = 0;

    return realSize;
}

void FetchWeather::showJSONdata() const {
    std::cout << json_data << std::endl;
}

// Updates distance between subsctring
// str - whole string; what - substring to search for; b - begin; e - end
// Strings are put in shuch format: "name":"Vilnius", ==> Vilnius
void find_distance(const std::string &str, const std::string &what, std::string::size_type &b, std::string::size_type &e){
    b = str.find(what);
    b += what.length() + 3; // 3 = ":"
    e = b;
    e = str.find('\"', e);
}

// Because numbers are put in such format: "temp_f":86.0," ==> 86.0
void find_distance_numbers(const std::string &str, const std::string &what, std::string::size_type &b, std::string::size_type &e){
    b = str.find(what);
    b += what.length() + 2; // 2 = ":
    e = b;
    e = str.find(',', e);
}

// Determines the wind direction
const std::string wind_dir(const std::string &str){
    if(str == "N")
        return "↑";
    else if(str == "NNE" || str == "NE" || str == "ENE")
        return "↗";
    else if(str == "E")
        return "→";
    else if(str == "ESE" || str == "SE" || str == "SSE")
        return "↘";
    else if(str == "S")
        return "↓";
    else if(str == "SSW" || str == "SW" || str == "WSW")
        return "↙";
    else if(str == "W")
        return "←";
    else if(str == "WNW" || str == "NW" || str == "NNW")
        return "↖";
    return "?";
}

void FetchWeather::parseJSON() {

    // Marks beginning and ending of a substring
    std::string::size_type begin = 0, end = 0;

    // Find city's name
    find_distance(json_data, "name", begin, end);
    current_data->city = json_data.substr(begin, end - begin);

    // Find country
    find_distance(json_data, "country", begin, end);
    current_data->country = json_data.substr(begin, end - begin);

    // Find when was the database last time updated
    find_distance(json_data, "last_updated\"", begin, end);
    current_data->last_updated = json_data.substr(begin-1, end - begin+1);

    // Find temperature
    find_distance_numbers(json_data, "temp_c", begin, end);
    current_data->temp_c = json_data.substr(begin, end - begin);
    current_data->temp_c += " °C";

    // Find condition
    find_distance(json_data, "text", begin, end);
    current_data->condition = json_data.substr(begin, end - begin);

    // Find wind m/s and its direction
    std::string wind_ms, dir;
    // 1) wind_ms
    find_distance_numbers(json_data, "wind_kph", begin, end);
    wind_ms = json_data.substr(begin, end - begin);
    wind_ms = std::to_string(std::atof(wind_ms.c_str()) * 1000 / 3600) + " m/s";
    // 2) find wind dir
    find_distance(json_data, "wind_dir", begin, end);
    dir = json_data.substr(begin, end - begin);
    current_data->wind = wind_dir(dir) + " " + wind_ms;

    json_data = json_data.substr(end, json_data.length() - end);
    end = begin = 0;

    // Parsing forecast data (all the other days)
    for(int day = 0; day < 6; day++){
        // Find date
        find_distance(json_data, "date\"", begin, end);
        forecast_data[day].date = json_data.substr(begin-1, end - begin+1);

        // Find max temperature
        find_distance_numbers(json_data, "maxtemp_c", begin, end);
        forecast_data[day].max_temp_c = json_data.substr(begin, end - begin);
        forecast_data[day].max_temp_c += " °C";

        // Find min temperature
        find_distance_numbers(json_data, "mintemp_c", begin, end);
        forecast_data[day].min_temp_c = json_data.substr(begin, end - begin);
        forecast_data[day].min_temp_c += " °C";

        // Wind
        wind_ms.clear();
        find_distance_numbers(json_data, "maxwind_kph", begin, end);
        wind_ms = json_data.substr(begin, end - begin);
        wind_ms = std::to_string(std::atof(wind_ms.c_str()) * 1000 / 3600) + " m/s";
        forecast_data[day].wind = wind_ms;

        // Find condition
        find_distance(json_data, "text", begin, end);
        forecast_data[day].condition = json_data.substr(begin, end - begin);

        // Find prescription in mm
        find_distance_numbers(json_data, "totalprecip_mm", begin, end);
        forecast_data[day].precip_mm = json_data.substr(begin, end - begin) + " mm";

        // Find sunrise
        find_distance(json_data, "sunrise\"", begin, end);
        forecast_data[day].sunrise = json_data.substr(begin-1, end - begin+1);

        // Find sunset
        find_distance(json_data, "sunset\"", begin, end);
        forecast_data[day].sunset = json_data.substr(begin-1, end - begin+1);

        json_data = json_data.substr(end, json_data.length() - end);
        end = begin = 0;
    }
}

// Simply displays the weather report
void FetchWeather::displayWeather() {
    // Current weather:
    std::cout << "Location:     " << current_data->city << ", " << current_data->country << std::endl;
    std::cout << "Last updated: " << current_data->last_updated << std::endl << std::endl;
    std::cout << "Current weather:" << std::endl;
    std::cout << Conditions::Cloudy::c[0] << std::endl;
    std::cout << Conditions::Cloudy::c[1] << " 🌡   " << current_data->temp_c << std::endl;
    std::cout << Conditions::Cloudy::c[2] << " 🌬 " <<  current_data->wind << std::endl;
    std::cout << Conditions::Cloudy::c[3] << std::endl;
    std::cout << Conditions::Cloudy::c[4] << std::endl;

    // Weather report for 6 days:
    std::vector<int> _ind = {0, 1, 2};
    for(int i = 0; i<2; i++){
        std::cout << "-------------------------------------------------------------------------------------------------------------------------\n";
        std::cout << "| " << std::internal << forecast_data[_ind[0]].date << std::setw(30) << " | " << forecast_data[_ind[1]].date << std::setw(30) << " | " << forecast_data[_ind[2]].date << std::setw(30) << "|\n";
        std::cout << "-------------------------------------------------------------------------------------------------------------------------\n";
        std::cout << "| " << Conditions::Cloudy::c[0] << " 🌡 " << std::left << std::setw(7) <<  forecast_data[_ind[0]].min_temp_c << std::setw(3) << " → " << std::setw(11) << forecast_data[_ind[0]].max_temp_c << " | "
                          << Conditions::Cloudy::c[0] << " 🌡 " << std::left << std::setw(7) <<  forecast_data[_ind[1]].min_temp_c << std::setw(3) << " → " << std::setw(11) << forecast_data[_ind[1]].max_temp_c << " | "
                          << Conditions::Cloudy::c[0] << " 🌡 " << std::left << std::setw(7) <<  forecast_data[_ind[2]].min_temp_c << std::setw(3) << " → " << std::setw(11) << forecast_data[_ind[2]].max_temp_c << " |" << std::endl;

        std::cout << "| " << Conditions::Cloudy::c[1] << " 🌬 " << std::left << std::setw(20) << forecast_data[_ind[0]].wind <<  " | "
                          << Conditions::Cloudy::c[1] << " 🌬 " << std::left << std::setw(20) << forecast_data[_ind[1]].wind <<  " | "
                          << Conditions::Cloudy::c[1] << " 🌬 " << std::left << std::setw(20) << forecast_data[_ind[2]].wind <<  " |" << std::endl;


        std::cout << "| " << Conditions::Cloudy::c[2] << " 🌧 " << std::left << std::setw(20) << forecast_data[_ind[0]].precip_mm << " | "
                          << Conditions::Cloudy::c[2] << " 🌧 " << std::left << std::setw(20) << forecast_data[_ind[1]].precip_mm << " | "
                          << Conditions::Cloudy::c[2] << " 🌧 " << std::left << std::setw(20) << forecast_data[_ind[2]].precip_mm << " |" << std::endl;


        std::cout << "| " << Conditions::Cloudy::c[3] << " ☀️  " << std::left << std::setw(20) << forecast_data[_ind[0]].sunrise << " | "
                          << Conditions::Cloudy::c[3] << " ☀️  " << std::left << std::setw(20) << forecast_data[_ind[1]].sunrise << " | "
                          << Conditions::Cloudy::c[3] << " ☀️  " << std::left << std::setw(20) << forecast_data[_ind[2]].sunrise << " |" << std::endl;


        std::cout << "| " << Conditions::Cloudy::c[4] << " 🌫 " << std::left << std::setw(20) << forecast_data[_ind[0]].sunset << " | "
                          << Conditions::Cloudy::c[4] << " 🌫 " << std::left << std::setw(20) << forecast_data[_ind[1]].sunset << " | "
                          << Conditions::Cloudy::c[4] << " 🌫 " << std::left << std::setw(20) << forecast_data[_ind[2]].sunset << " |" << std::endl;

        std::cout << "-------------------------------------------------------------------------------------------------------------------------\n\n";
        _ind.clear(); _ind = {3, 4, 5}; // put new indexes
    }
}