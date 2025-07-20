#ifndef DBENGINE_H
    #define DBENGINE_H

#ifdef _WIN32
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT __attribute__((visibility("default")))
#endif

class DLLEXPORT DataBaseEngine {
  public:
    void initDBEngine() const;
};

#endif