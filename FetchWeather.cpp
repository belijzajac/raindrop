#include "FetchWeather.h"

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
    delete currect_data;
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

void FetchWeather::parseJSON() {

    // Marks beginning and ending of a substring
    std::string::size_type begin, end;

    // Finds city's name
    find_distance(json_data, "name", begin, end);
    currect_data->name = json_data.substr(begin, end - begin);
    std::cout << currect_data->name << std::endl;

    // Finds country
    find_distance(json_data, "country", begin, end);
    currect_data->country = json_data.substr(begin, end - begin);
    std::cout << currect_data->country << std::endl;

    // Find when was the database last time updated
    find_distance(json_data, "last_updated\"", begin, end);
    currect_data->last_updated = json_data.substr(begin-1, end - begin+1);
    std::cout << currect_data->last_updated << std::endl;

    // Find temperature
    find_distance_numbers(json_data, "temp_c", begin, end);
    currect_data->temp_c = json_data.substr(begin, end - begin);
    currect_data->temp_c += " °C";
    std::cout << currect_data->temp_c << std::endl;

    // Find condition
    find_distance(json_data, "text", begin, end);
    currect_data->condition = json_data.substr(begin, end - begin);
    std::cout << currect_data->condition << std::endl;

    // Find wind m/s and its direction
    std::string wind_ms, dir;
    // 1) wind_ms
    find_distance_numbers(json_data, "wind_kph", begin, end);
    wind_ms = json_data.substr(begin, end - begin);

    /* TODO: */

    //wind_ms = std::to_string( std::fixed << std::setprecision(2) << (std::atof(wind_ms.c_str()) * 1000 / 3600)) + " m/s";
    std::cout << wind_ms << std::endl;
    // 2) find wind dir
    find_distance(json_data, "wind_dir", begin, end);
    dir = json_data.substr(begin, end - begin);
    std::cout << dir << std::endl;
}