#include "../public_api.hpp"

ExecutionContext::ExecutionContext() {
  LoggerService::StatusLogger::printAsStandardOutput("Initializing Execution Context...");
  auto start = std::chrono::steady_clock::now();

  ExecutionContext::initializeUntypedTableMetadata();
  ExecutionContext::initializeColumnEncodingMap();
  ExecutionContext::initializePrimaryKeyIndeces();
  // TODO initialize indeces

  auto end = std::chrono::steady_clock::now();
  std::chrono::duration<double, std::milli> double_duration = end - start;
  LoggerService::StatusLogger::printAsStandardOutput("Execution context initialized in " +
                                                     std::to_string(double_duration.count()) + " ms");
}
