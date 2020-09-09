#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>

#include "diskarray.h"

using namespace std;

int parseFile(char *path, char *buf, int size)
{
    FILE *pFile = NULL;
    long FileSize = 0;

    pFile = fopen(path, "r");
    if (pFile == NULL) {
        printf("Read file failed: %s\n", path);
        goto fail;
    }
    fread(buf, 1, size, pFile);
    fclose(pFile);
    return 0;

fail:
    if (pFile != NULL) {
        fclose(pFile);
    }
    return -1;
}

uint64_t Diskarray::parseArraysize(void)
{
    if (this->level == "linear") {
        this->ArraySize = this->minDiskSize * this->Disks.size();
    }
    else if (this->level == "raid1") {
        this->ArraySize = this->minDiskSize;
    }
    else if (this->level == "raid5") {
        this->ArraySize = this->minDiskSize * (this->Disks.size() - 1);
    }
    else if (this->level == "raid6") {
        this->ArraySize = this->minDiskSize * (this->Disks.size() - 2);
    }
    else {
        perror("non-supported level\n");
        return 0;
    }
    return this->ArraySize;
}

Diskarray::Diskarray(string md) : mdName(md), minDiskSize(UINT64_MAX)
{
    DIR *dir = NULL;
    struct dirent *ent = NULL;
    uint32_t offset = 0;
    uint64_t disksize = 0;
    char slavesPath[128] = {0};
    char buffer[32] = {0};
    FILE *File = NULL;
    char Path[128] = {0};
    long FileSize = 0;
    
    sprintf(slavesPath, SYSFS_MD_SLAVE_FORMAT, md.c_str());
    if ((dir = opendir (slavesPath)) != NULL) {
        /* print all the files and directories within directory */
        while ((ent = readdir (dir)) != NULL) {
            memset(Path, 0, 128);
            memset(buffer, 0, 32);
            sprintf(Path, SYSFS_MD_DISK_OFFSET_FORMAT, md.c_str(), ent->d_name);
            if (parseFile(Path, buffer, 32) < 0) {
                continue;
            }
            sscanf(buffer, "%lu", &offset);
            this->Disks.push_back(Disk(ent->d_name, offset));

            memset(Path, 0, 128);
            memset(buffer, 0, 32);
            sprintf(Path, SYSFS_MD_DISK_SIZE_FORMAT, md.c_str(), ent->d_name);
            if (parseFile(Path, buffer, 32) < 0) {
                continue;
            }
            sscanf(buffer, "%lu", &disksize);
            if (this->minDiskSize > disksize) {
                this->minDiskSize = disksize;
            }
        }
        closedir (dir);
    }
    else {
        perror("Open slaves directory, failed\n");
    }

    memset(Path, 0, 128);
    memset(buffer, 0, 32);
    sprintf(Path, SYSFS_MD_CHUNK_SIZE_FORMAT, md.c_str());
    if (parseFile(Path, buffer, 32) < 0) {
        goto fail;
    }
    sscanf(buffer, "%lu", &this->chunkSize);

    memset(Path, 0, 128);
    memset(buffer, 0, 32);
    sprintf(Path, SYSFS_MD_LEVEL_FORMAT, md.c_str());
    if (parseFile(Path, buffer, 32) < 0) {
        goto fail;
    }
    this->level = buffer;
    this->level.erase(this->level.end() - 1);

    this->parseArraysize();
fail:
    return;
}

void Diskarray::PrintDiskarray(void)
{
    vector<Disk>::iterator itrDisk;
    printf("md name: %s\n", this->mdName.c_str());
    printf("chunk size: %lu\n", this->chunkSize);
    printf("minimum disk size: %llu\n", this->minDiskSize);
    printf("Array size: %llu\n", this->ArraySize);
    printf("level: %s\n", this->level.c_str());

    printf("Disks:\n");
    for (itrDisk = this->Disks.begin(); itrDisk != this->Disks.end(); itrDisk++) {
        printf("\t");
        itrDisk->PrintDisk();
    }
}

int Diskarray::GetDiskSector(uint32_t sector, uint32_t *diskSector)
{
    if (this->level == "linear") {
        return this->GetDiskSectorLinear(sector, diskSector);
    }
    else if (this->level == "raid1") {
        return this->GetDiskSectorRaid1(sector, diskSector);
    }
    else if (this->level == "raid5") {
        return this->GetDiskSectorRaid5(sector, diskSector);
    }
    else if (this->level == "raid6") {
        return this->GetDiskSectorRaid6(sector, diskSector);
    }
    else {
        perror("Non-supported raid level\n");
        return -1;
    }
}

int Diskarray::GetDiskSectorLinear(uint32_t sector, uint32_t *diskSector)
{
}

int Diskarray::GetDiskSectorRaid1(uint32_t sector, uint32_t *diskSector)
{
}

int Diskarray::GetDiskSectorRaid5(uint32_t sector, uint32_t *diskSector)
{
}

int Diskarray::GetDiskSectorRaid6(uint32_t sector, uint32_t *diskSector)
{
}

Disk::Disk(string name, uint32_t offset) : name(name), offset(offset)
{}

void Disk::PrintDisk()
{
    printf("Disk name: %s, Offset: %lu\n", this->name.c_str(), this->offset);
}
