/* Copyright (c) Synology Inc. All rights reserved.*/

#ifndef SYNOMDMAPPER_H
#define SYNOMDMAPPER_H

#include <string>
#include "diskarray.h"

using namespace std;

#define SYSFS_PATH "/sys/block/"

class Mapper {
public:
    Mapper(string ops, string md, string disk, int qsector);
    void printMapping(void);
    void printReverseMapping(void);

private:
    string operation;
    string mdName, diskName;
    uint32_t querySector;
    Diskarray diskarray;
};

#endif
