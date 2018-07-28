#ifndef RAINDROP_FETCHWEATHER_H
#define RAINDROP_FETCHWEATHER_H

#include <curl/curl.h>
#include <string>

// Struct that holds weather info
struct Data{
    std::string name;
    std::string last_updated;
    std::string condition;
    std::string wind_dir;
    float temp_c;
    float wind_mph;
};

class FetchWeather{
private:
    // Extracted data
    std::string json_data;

    // Private memory struct
    struct MemoryStruct {
        char *memory{};
        size_t size;

        MemoryStruct() : size(0) {}
    };

public:
    explicit FetchWeather(const std::string &url);
    ~FetchWeather() = default;

    static size_t writeMemoryCallback(void *contents, size_t size, size_t nmemb, void *buffer_in);
    void showJSONdata() const;
};

#endif //RAINDROP_FETCHWEATHER_H
