#pragma once
#include <iostream>
#include <string>
// Static method and module container
struct LoggerService {

  LoggerService() = delete;
  LoggerService(const LoggerService&) = delete;
  LoggerService& operator=(const LoggerService&) = delete;

  struct ErrorLogger {
    ErrorLogger() = delete;
    ErrorLogger(const ErrorLogger&) = delete;
    ErrorLogger& operator=(const ErrorLogger&) = delete;

    static void printAsStandardError(const std::string& error_msg);
  };

  struct StatusLogger {
    StatusLogger() = delete;
    StatusLogger(const StatusLogger&) = delete;
    StatusLogger& operator=(const StatusLogger&) = delete;

    static void printAsStandardOutput(const std::string& status_msg);
  };
};