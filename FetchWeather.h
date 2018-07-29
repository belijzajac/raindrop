#ifndef RAINDROP_FETCHWEATHER_H
#define RAINDROP_FETCHWEATHER_H

#include <curl/curl.h>
#include <string>

// Struct that holds weather info
struct Data{
    std::string name;
    std::string country;
    std::string last_updated;
    std::string condition;
    std::string wind_dir;
    std::string temp_c;
    std::string wind_mph;
};

class FetchWeather{
private:
    // Extracted data
    std::string json_data;

    struct Data *currect_data = new Data;

    // Private memory struct
    struct MemoryStruct {
        char *memory{};
        size_t size;

        MemoryStruct() : size(0) {}
    };

public:
    explicit FetchWeather(const std::string &url);
    ~FetchWeather();

    static size_t writeMemoryCallback(void *contents, size_t size, size_t nmemb, void *buffer_in);
    void showJSONdata() const;
    void parseJSON();
};

#endif //RAINDROP_FETCHWEATHER_H
