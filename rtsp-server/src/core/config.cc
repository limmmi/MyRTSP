#include "core/config.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <getopt.h>

Config& Config::GetInstance() {
        static Config instance;
        return instance;
}

void Config::ParseCommandLine(int argc, char** argv) {
    // TODO: Implement command line parsing logic
}

bool ServerConfig::LoadFromFile(const std::string& filename) {
    // TODO: Implement file loading logic
    return true;
}

bool ServerConfig::SaveToFile(const std::string& filename) {
    // TODO: Implement file saving logic
    return true;
}
