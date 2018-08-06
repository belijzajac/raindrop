#ifndef RAINDROP_UTILITIES_H
#define RAINDROP_UTILITIES_H

#include <pwd.h>
#include <zconf.h>
#include <string>

// Static class of useful utilities
class Utilities{
public:
    // Gets username of current user working on Linux
    static std::string getUsername(){
        uid_t uid = geteuid();
        struct passwd *pw = getpwuid(uid);
        if(pw){
            return std::string(pw->pw_name);
        }
        return {};
    }

private:
    // Disallow creating an instance of this object
    Utilities() = default;
};

#endif //RAINDROP_UTILITIES_H
