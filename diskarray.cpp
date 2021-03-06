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
    int slot = -1;
    char slavesPath[128] = {0};
    char buffer[32] = {0};
    FILE *File = NULL;
    char Path[128] = {0};
    long FileSize = 0;
    vector<Disk> _Disks;
    
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

            memset(Path, 0, 128);
            memset(buffer, 0, 32);
            sprintf(Path, SYSFS_MD_DISK_SLOT_FORMAT, md.c_str(), ent->d_name);
            if (parseFile(Path, buffer, 32) < 0) {
                continue;
            }
            sscanf(buffer, "%d", &slot);

            _Disks.push_back(Disk(ent->d_name, offset, slot));

            memset(Path, 0, 128);
            memset(buffer, 0, 32);
            sprintf(Path, SYSFS_MD_DISK_SIZE_FORMAT, md.c_str(), ent->d_name);
            if (parseFile(Path, buffer, 32) < 0) {
                continue;
            }
            sscanf(buffer, "%lu", &disksize);
            disksize = disksize * 1024;
            if (this->minDiskSize > disksize) {
                this->minDiskSize = disksize;
            }
        }
        closedir (dir);
    }
    else {
        perror("Open slaves directory, failed\n");
    }

    this->Disks = vector<Disk>(_Disks);
    for (auto disk : _Disks) {
        this->Disks[disk.slot] = disk;
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
    int disk = -1;
    if (this->level == "linear") {
        disk = this->GetDiskSectorLinear(sector, diskSector);
    }
    else if (this->level == "raid1") {
        disk = this->GetDiskSectorRaid1(sector, diskSector);
    }
    else if (this->level == "raid5") {
        disk = this->GetDiskSectorRaid5(sector, diskSector);
    }
    else if (this->level == "raid6") {
        disk = this->GetDiskSectorRaid6(sector, diskSector);
    }
    else {
        perror("Non-supported raid level\n");
        goto fail;
    }

    if (disk < 0) {
        printf("[error]: %s\n", __FUNCTION__);
        goto fail;
    }
    *diskSector = *diskSector + this->Disks[disk].offset;
    return disk;

fail:
    return -1;
}

int Diskarray::GetDiskSectorLinear(uint32_t sector, uint32_t *diskSector)
{
    int disk = -1;
    uint32_t sec = 0;
    uint32_t sectorsInDisk = 0;

    if (sector * SECTOR_SIZE > this->ArraySize) {
        perror("Linear, out of boundary\n");
        goto fail;
    }

    sectorsInDisk = this->minDiskSize / this->chunkSize * this->chunkSize / SECTOR_SIZE;
    disk = sector / sectorsInDisk;
    sec = sector % sectorsInDisk;
    *diskSector = sec;
    return disk;
    
fail:
    return -1;
}

int Diskarray::GetDiskSectorRaid1(uint32_t sector, uint32_t *diskSector)
{
    int disk = -1;
    uint32_t sec = 0;

    if (sector * SECTOR_SIZE > this->ArraySize) {
        perror("Raid1, out of boundary\n");
        goto fail;
    }

    disk = 0;
    *diskSector = sector;
    return disk;

fail:
    return -1;
}

int Diskarray::GetDiskSectorRaid5(uint32_t sector, uint32_t *diskSector)
{
    uint32_t sectorsInChunk = 0;
    uint32_t sectorsInStrip = 0;
    uint32_t chunk = 0;
    int disk = -1;

    if (sector * SECTOR_SIZE > this->ArraySize) {
        perror("Raid5, out of boundary\n");
        goto fail;
    }

    sectorsInChunk = this->chunkSize / SECTOR_SIZE;
    sectorsInStrip = sectorsInChunk * (this->Disks.size() - 1);

    chunk = sector / sectorsInChunk;
    disk = chunk % this->Disks.size();
    *diskSector = sector % (sectorsInChunk) + sector / sectorsInStrip * sectorsInChunk;

    return disk;
fail:
    return -1;
}

int Diskarray::GetDiskSectorRaid6(uint32_t sector, uint32_t *diskSector)
{
    int disk = -1;
    uint64_t sectorsInChunk = 0;
    uint64_t chunkOnMd = 0;
    uint64_t groupOnMd = 0;
    uint64_t offsetInChunk = 0;
    uint64_t numInGroup = 0; // chunk
    uint64_t chunksInGroup = 0;
    uint64_t diskChunk = 0;
    uint64_t mod = 0, quotient = 0, threshold = 0;
    if (sector * SECTOR_SIZE > this->ArraySize) {
        perror("Raid6, out of boundary\n");
        goto fail;
    }

    chunksInGroup = (this->Disks.size() - 2) * this->Disks.size();
    sectorsInChunk = this->chunkSize / SECTOR_SIZE;
    chunkOnMd = sector / sectorsInChunk;
    offsetInChunk = sector % sectorsInChunk;

    /*
     * Firstly, figure out which group the chunk belonged to and the offset chunk in the Group
     * groupOnMd and numInGroup seperately
     * Secondly, see this in a disk view.
     * Since the life span is the number of disks a cycle. This gives number of disks chunks a group.
     *  this is why we got: this->Disks.size() * groupOnMd.
     * Then, each strip contains number of disks - 2 chunks
     *  this is why we got: numInGroup / (this->Disks.size() - 2)
     * 
     */
    groupOnMd = chunkOnMd / chunksInGroup;
    numInGroup = chunkOnMd % chunksInGroup;
    diskChunk = this->Disks.size() * groupOnMd + numInGroup / (this->Disks.size() - 2);
    *diskSector = diskChunk * sectorsInChunk + offsetInChunk;

    /*
     * Example:
     * layout:
     * x  0  1  2  x
     * 3  4  5  x  x
     * 7  8  x  x  6
     * 11 x  x  9  10
     * x  x  12 13 14
     *
     * See this like:
     *(-1)(2)(1)(0) <- threshold
     *  x  0  1  2  (0)
     *  3  4  5  6  (1)
     *  7  8  9  10 (2)
     *  11 12 13 14 (3)
     *               ^
     *             quotient
     *
     */
    mod = numInGroup % (this->Disks.size() - 1);
    quotient = numInGroup / (this->Disks.size() - 1);
    disk = (mod + 1) % (this->Disks.size() - 1);
    threshold = this->Disks.size() - 3 - mod;
    if (threshold >= 0 && quotient > threshold) {
        disk++;
    }
    return disk;

fail:
    return -1;
}

Disk Diskarray::GetDiskFromName(string diskName)
{
    Disk queryDisk("", 0, 0);
    for (auto disk : this->Disks) {
        if (diskName == disk.name) {
            queryDisk = disk;
        }
    }
    return queryDisk;
}

uint64_t Diskarray::GetRaidSector(string diskName, uint32_t sector)
{
    uint64_t querySector = 0;
    Disk queryDisk = Disk("", 0, 0);
    uint64_t mdSector = 0;

    queryDisk = this->GetDiskFromName(diskName);
    querySector = sector - queryDisk.offset;

    if (this->level == "linear") {
        mdSector = this->GetRaidSectorLinear(queryDisk, querySector);
    }
    else if (this->level == "raid1") {
        mdSector = this->GetRaidSectorRaid1(queryDisk, querySector);
    }
    else if (this->level == "raid5") {
        mdSector = this->GetRaidSectorRaid5(queryDisk, querySector);
    }
    else if (this->level == "raid6") {
        mdSector = this->GetRaidSectorRaid6(queryDisk, querySector);
    }
    else {
        perror("Non-supported raid level\n");
        goto fail;
    }

    return mdSector;
fail:
    return UINT64_MAX;
}

uint64_t Diskarray::GetRaidSectorLinear(Disk disk, uint64_t sector)
{
    uint64_t sectorsInDisk;

    sectorsInDisk = this->minDiskSize / this->chunkSize * this->chunkSize / SECTOR_SIZE;
    return disk.slot * sectorsInDisk + sector;
}

uint64_t Diskarray::GetRaidSectorRaid1(Disk disk, uint64_t sector)
{
    return sector;
}

uint64_t Diskarray::GetRaidSectorRaid5(Disk disk, uint64_t sector)
{
    uint64_t sectorsInChunk = 0;
    uint64_t chunkOnDisk = 0;
    uint64_t offsetInChunk = 0;
    uint64_t chunksInGroup = 0;
    uint64_t group = 0;
    uint64_t diskchunkInGroup = 0;
    uint64_t chunkOnMd = 0;
    uint64_t sectorOnMd = 0;

    sectorsInChunk = this->chunkSize / SECTOR_SIZE;
    chunksInGroup = (this->Disks.size() - 1) * this->Disks.size();

    chunkOnDisk = sector / sectorsInChunk;
    offsetInChunk = sector % sectorsInChunk;
    group = chunkOnDisk / this->Disks.size();
    diskchunkInGroup = chunkOnDisk % this->Disks.size();

    if (disk.slot + diskchunkInGroup == this->Disks.size() - 1) {
        perror("This is a parity chunk\n");
        goto fail;
    }
    else if (disk.slot + diskchunkInGroup > this->Disks.size() - 1) {
        chunkOnMd = group * chunksInGroup + this->Disks.size() * (diskchunkInGroup - 1) + disk.slot;
    }
    else {
        chunkOnMd = group * chunksInGroup + this->Disks.size() * diskchunkInGroup + disk.slot;
    }
    sectorOnMd = chunkOnMd * sectorsInChunk + offsetInChunk;

    return sectorOnMd;

fail:
    return UINT64_MAX;
}

uint64_t Diskarray::GetRaidSectorRaid6(Disk disk, uint64_t sector)
{
    uint64_t sectorsInChunk = 0;
    uint64_t chunksInGroup = 0;
    uint64_t chunkOnDisk = 0;
    uint64_t offsetInChunk = 0;
    uint64_t group = 0;
    uint64_t chunkOnMd = 0;
    uint64_t offsetInGroup = 0;
    uint64_t sectorOnMd = 0;
    int pdisk = 0, qdisk = 0;

    if (sector * SECTOR_SIZE > this->minDiskSize) {
        perror("Raid 6 reverse mapping, out of boundary\n");
        goto fail;
    }

    sectorsInChunk = this->chunkSize / SECTOR_SIZE;
    chunksInGroup = (this->Disks.size() - 2) * this->Disks.size();
    chunkOnDisk = sector / sectorsInChunk;
    offsetInChunk = sector % sectorsInChunk;
    group = chunkOnDisk / this->Disks.size();
    chunkOnDisk = chunkOnDisk % this->Disks.size();

    pdisk = (this->Disks.size() - 1) - chunkOnDisk;
    qdisk = (pdisk + 1) % this->Disks.size();
    if (pdisk == disk.slot || qdisk == disk.slot) {
        perror("This is a parity chunk\n");
        goto fail;
    }
    offsetInGroup = chunkOnDisk * (this->Disks.size() - 2) + (disk.slot + this->Disks.size() - qdisk - 1) % this->Disks.size();

    chunkOnMd = group * chunksInGroup + offsetInGroup;
    sectorOnMd = chunkOnMd * sectorsInChunk + offsetInChunk;
    return sectorOnMd;
fail:
    return UINT64_MAX;
}

Disk::Disk(string name, uint32_t offset, int slot) : name(name), offset(offset), slot(slot)
{}

void Disk::PrintDisk()
{
    printf("[%d]:Disk name=%s; Offset=%lu\n", this->slot, this->name.c_str(), this->offset);
}
