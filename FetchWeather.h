#ifndef RAINDROP_FETCHWEATHER_H
#define RAINDROP_FETCHWEATHER_H

#include <curl/curl.h>
#include <string>
#include <vector>

// Holds weather info for the current day
struct curr_day{
    std::string city,
                country,
                last_updated,
                condition,
                wind,
                temp_c;
    std::vector<std::string> ascii_art;
};
typedef struct curr_day curr_day;

// Holds forecast data
struct forecast{
    std::string date,
                condition,
                wind,
                max_temp_c,
                min_temp_c,
                precip_mm,
                sunrise,
                sunset;
    std::vector<std::string> ascii_art;
};
typedef struct forecast forecast;

class FetchWeather{
private:
    // Extracted data
    std::string json_data;

    curr_day *current_data  = new curr_day;
    forecast *forecast_data = new forecast[6];

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
    void displayWeather() const;
    const std::vector<std::string> get_condition(const std::string &cond) const;
    friend const std::string fix_wind_floating_point(const std::string &str);
};

#endif //RAINDROP_FETCHWEATHER_H
