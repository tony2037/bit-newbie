/* Copyright (c) Synology Inc. All rights reserved.*/
#ifndef DISKARRAY_H
#define DISKARRAY_H

#include <stdint.h>
#include <string>
#include <vector>

using namespace std;

#define SYSFS_MD_FORMAT "/sys/block/%s/"
#define SYSFS_MD_CHUNK_SIZE_FORMAT "/sys/block/%s/md/chunk_size"
#define SYSFS_MD_LEVEL_FORMAT "/sys/block/%s/md/level"
#define SYSFS_MD_DISK_OFFSET_FORMAT "/sys/block/%s/md/dev-%s/offset"
#define SYSFS_MD_SLAVE_FORMAT "/sys/block/%s/slaves/"

class Disk {
public:
    Disk(string name, uint32_t offset);
    void PrintDisk();

private:
    string name;
    uint32_t offset;
};

class Diskarray {
public:
    Diskarray(string md);
    void PrintDiskarray();

private:
    string mdName;
    vector<Disk> Disks;
    uint32_t chunkSize;
    string level;
};

int parseFile(char *, char *);
#endif
