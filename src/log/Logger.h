//
// Created by py_01 on 26-7-24.
//

#ifndef LOGGER_H
#define LOGGER_H

#include<iostream>

class Logger {
public:
    static void info(const std::string& msg) {
        std::cout << "[INFO] " << msg << std::endl;
    }
    static void error(const std::string& msg) {
        std::cerr << "[ERROR] " << msg << std::endl;
    }
    static void debug(const std::string& msg) {
        std::cout << "[DEBUG] " << msg << std::endl;
    }
};

#endif //LOGGER_H
