#include "FetchWeather.h"

#include <curl/curl.h>
#include <curl/easy.h>

#include <iostream>
#include <cstring>
#include <sstream>

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