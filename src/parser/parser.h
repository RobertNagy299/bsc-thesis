#ifndef SQLPARSER_H
    #define SQLPARSER_H

#ifdef _WIN32
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT __attribute__((visibility("default")))
#endif

class DLLEXPORT SQLParser {
  public:
    void initParser() const;
};

#endif
