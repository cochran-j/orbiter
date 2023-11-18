// Copyright (c) Martin Schweiger
// Licensed under the MIT License

#ifndef __MEMSTAT_H
#define __MEMSTAT_H

#include <memory>

class MemStat_impl;

class MemStat {
public:
    MemStat ();
    ~MemStat();

    MemStat(MemStat&&);
    MemStat& operator=(MemStat&&);

    long HeapUsage ();

private:
    std::unique_ptr<MemStat_impl> pImpl;
};

#endif // !__MEMSTAT_H
