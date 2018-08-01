#include "FetchWeather.h"
#include "Conditions.h"

#include <curl/curl.h>
#include <curl/easy.h>

#include <iostream>
#include <cstring>
#include <cmath>
#include <iomanip>

// Constructor
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

// Destructor
FetchWeather::~FetchWeather() {
    delete current_data;
    delete [] forecast_data;
}

// libCURL needs it to write data constantly to memory
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

// Updates distance between substring
// str - whole string; what - substring to search for; b - begin; e - end
// Strings are put in such format: "name":"Vilnius", ==> Vilnius
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

const std::string fix_wind_floating_point(const std::string &str){
    std::string::size_type point = str.find('.'); // find how much to substract
    return str.substr(0, point + 2);              // 2 ==> '.' + 1 extra digit
}

// Parses JSON
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
    current_data->ascii_art = get_condition(current_data->condition);

    // Find wind m/s and its direction
    std::string wind_ms, dir;
    // 1) wind_ms
    find_distance_numbers(json_data, "wind_kph", begin, end);
    wind_ms = json_data.substr(begin, end - begin);
    wind_ms = std::to_string(std::atof(wind_ms.c_str()) * 1000 / 3600);
    wind_ms = fix_wind_floating_point(wind_ms) + " m/s";
    // 2) find wind dir
    find_distance(json_data, "wind_dir", begin, end);
    dir = json_data.substr(begin, end - begin);
    current_data->wind = wind_ms + " " + wind_dir(dir);

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
        wind_ms = std::to_string(std::atof(wind_ms.c_str()) * 1000 / 3600);
        wind_ms = fix_wind_floating_point(wind_ms) + " m/s";
        forecast_data[day].wind = wind_ms;

        // Find condition
        find_distance(json_data, "text", begin, end);
        forecast_data[day].condition = json_data.substr(begin, end - begin);
        forecast_data[day].ascii_art = get_condition(forecast_data[day].condition);

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

// Gets corresponding ASCII art image per provided weather condition
const std::vector<std::string> FetchWeather::get_condition(const std::string &cond) const{
    if(cond == "Cloudy")
        return Conditions::Cloudy::c;
    else if(cond == "Fog" || cond == "Freezing fog" || cond == "Mist")
        return Conditions::Fog::c;
    else if(cond == "Heavy rain at times" || cond == "Heavy rain" || cond == "Moderate rain at times" || cond == "Moderate rain" || cond == "Moderate or heavy freezing rain")
        return Conditions::HeavyRain::c;
    else if(cond == "Moderate or heavy rain shower" || cond == "Torrential rain shower")
        return Conditions::HeavyShowers::c;
    else if(cond == "Blowing snow" || cond == "Heavy snow" || cond == "Patchy heavy snow" || cond == "Moderate snow" || cond == "Heavy freezing drizzle")
        return Conditions::HeavySnow::c;
    else if(cond == "Moderate or heavy snow showers" || cond == "Blizzard")
        return Conditions::HeavySnowShowers::c;
    else if(cond == "Patchy rain possible" || cond == "Patchy light rain" || cond == "Light rain" || cond == "Light freezing rain")
        return Conditions::LightRain::c;
    else if(cond == "Light rain shower")
        return Conditions::LightShowers::c;
    else if(cond == "Patchy sleet possible" || cond == "Light sleet" || cond == "Moderate or heavy sleet")
        return Conditions::LightSleet::c;
    else if(cond == "Light sleet showers" || cond == "Moderate or heavy sleet showers")
        return Conditions::LightSleetShowers::c;
    else if(cond == "Patchy snow possible" || cond == "Patchy light snow" || cond == "Light snow" || cond == "Patchy moderate snow" || cond == "Patchy light drizzle" || cond == "Light drizzle" || cond == "Freezing drizzle" || cond == "Patchy freezing drizzle possible")
        return Conditions::LightSnow::c;
    else if(cond == "Light snow showers")
        return Conditions::LightSnowShowers::c;
    else if(cond == "Partly cloudy")
        return Conditions::PartlyCloudy::c;
    else if(cond == "Sunny" || cond == "Clear")
        return Conditions::Sunny::c;
    else if(cond == "Moderate or heavy rain with thunder")
        return Conditions::ThunderyHeavyRain::c;
    else if(cond == "Patchy light rain with thunder" || cond == "Thundery outbreaks possible")
        return Conditions::ThunderyShowers::c;
    else if(cond == "Patchy light snow with thunder" || cond == "Moderate or heavy snow with thunder")
        return Conditions::ThunderySnowShowers::c;
    else if(cond == "Overcast")
        return Conditions::VeryCloudy::c;
    else if(cond == "Ice pellets" || cond == "Light showers of ice pellets" || cond == "Moderate or heavy showers of ice pellets")
        return Conditions::IcePellets::c;

    return Conditions::Unknown::c;
}

// Simply displays the weather report
void FetchWeather::displayWeather() const {
    // Current weather:
    std::cout << "Location:     " << current_data->city << ", " << current_data->country << std::endl;
    std::cout << "Last updated: " << current_data->last_updated << std::endl << std::endl;
    std::cout << "Current weather:" << std::endl;
    std::cout << current_data->ascii_art[0] << " >  " <<  current_data->condition << std::endl;
    std::cout << current_data->ascii_art[1] << " 🌡 " << current_data->temp_c << std::endl;
    std::cout << current_data->ascii_art[2] << " 🌬 " <<  current_data->wind << std::endl;
    std::cout << current_data->ascii_art[3] << std::endl;
    std::cout << current_data->ascii_art[4] << std::endl;

    // Weather report for 6 days:
    std::vector<int> _ind = {0, 1};
    for(int i = 0; i<3; i++){
        std::cout << "-------------------------------------------------------------------------------------------------------------------------\n";
        std::cout << "|  " << "\e[1;32m" << forecast_data[_ind[0]].date << "\e[0m" << "  | " << std::setw(46) << "  |  " << "\e[1;32m" << forecast_data[_ind[1]].date << "\e[0m" << "  | " << std::setw(45) << "|\n";
        std::cout << "|--------------|" << std::setw(60) << "|--------------|" << std::setw(46) << "|\n";
        std::cout << "| " << forecast_data[_ind[0]].ascii_art[0] << " >  " << std::left << std::setw(40) << forecast_data[_ind[0]].condition << " | "
                          << forecast_data[_ind[1]].ascii_art[0] << " >  " << std::left << std::setw(40) << forecast_data[_ind[1]].condition << " |\n";

        std::cout << "| " << forecast_data[_ind[0]].ascii_art[1] << " 🌡 " << std::left << std::setw(7) <<  forecast_data[_ind[0]].min_temp_c << std::setw(3) << " ⇒ " << std::setw(11) << forecast_data[_ind[0]].max_temp_c << std::internal << std::setw(23) << " | "
                          << forecast_data[_ind[1]].ascii_art[1] << " 🌡 " << std::left << std::setw(7) <<  forecast_data[_ind[1]].min_temp_c << std::setw(3) << " ⇒ " << std::setw(11) << forecast_data[_ind[1]].max_temp_c << std::internal << std::setw(23) << " |\n";

        std::cout << "| " << forecast_data[_ind[0]].ascii_art[2] << " 🌬 " << std::left << std::setw(20) << forecast_data[_ind[0]].wind << std::internal << std::setw(23) << " | "
                          << forecast_data[_ind[1]].ascii_art[2] << " 🌬 " << std::left << std::setw(20) << forecast_data[_ind[1]].wind << std::internal << std::setw(23) << " |\n";

        std::cout << "| " << forecast_data[_ind[0]].ascii_art[3] << " 🌧 " << std::left << std::setw(20) << forecast_data[_ind[0]].precip_mm << std::internal << std::setw(23) << " | "
                          << forecast_data[_ind[1]].ascii_art[3] << " 🌧 " << std::left << std::setw(20) << forecast_data[_ind[1]].precip_mm << std::internal << std::setw(23) << " |\n";

        std::cout << "| " << forecast_data[_ind[0]].ascii_art[4] << " ☀️  " << std::left << std::setw(8) << forecast_data[_ind[0]].sunrise << " ⇒ " << std::setw(9) << forecast_data[_ind[0]].sunset << std::internal << std::setw(23) << " | "
                          << forecast_data[_ind[1]].ascii_art[4] << " ☀️  " << std::left << std::setw(8) << forecast_data[_ind[1]].sunrise << " ⇒ " << std::setw(9) << forecast_data[_ind[1]].sunset << std::internal << std::setw(23) << " |\n";
        _ind[0] += 2; _ind[1] += 2; // update indexes
    }
    std::cout << "-------------------------------------------------------------------------------------------------------------------------\n";
}