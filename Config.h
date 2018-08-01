#ifndef RAINDROP_CONFIG_H
#define RAINDROP_CONFIG_H

#include <string>

class Config{
private:
    std::string url;
    std::string m_key, m_city, m_days;
public:
    Config();
    ~Config() = default;

    void init();
    const std::string get_url() const;
};

#endif //RAINDROP_CONFIG_H
