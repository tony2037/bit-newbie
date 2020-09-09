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
#define SYSFS_MD_DISK_SIZE_FORMAT "/sys/block/%s/md/dev-%s/size"
#define SYSFS_MD_SLAVE_FORMAT "/sys/block/%s/slaves/"

#define SECTOR_SIZE 512

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

    void PrintDiskarray(void);

    uint64_t parseArraysize(void);

    int GetDiskSector(uint32_t sector, uint32_t *diskSector);
    int GetDiskSectorLinear(uint32_t sector, uint32_t *diskSector);
    int GetDiskSectorRaid1(uint32_t sector, uint32_t *diskSector);
    int GetDiskSectorRaid5(uint32_t sector, uint32_t *diskSector);
    int GetDiskSectorRaid6(uint32_t sector, uint32_t *diskSector);

private:
    string mdName;
    vector<Disk> Disks;
    uint32_t chunkSize;
    uint64_t minDiskSize;
    uint64_t ArraySize;
    string level;
};

int parseFile(char *, char *, int);
#endif
