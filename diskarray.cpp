#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>

#include "diskarray.h"

using namespace std;

int parseFile(char *path, char *buf)
{
    FILE *pFile = NULL;
    long FileSize = 0;

    pFile = fopen(path, "r");
    if (pFile == NULL) {
        printf("Read file failed: %s\n", path);
        goto fail;
    }
    fseek(pFile, 0, SEEK_END);
    FileSize = ftell(pFile);
    rewind(pFile);
    fread(buf, 1, FileSize, pFile);
    fclose(pFile);
    return 0;

fail:
    if (pFile != NULL) {
        fclose(pFile);
    }
    return -1;
}

Diskarray::Diskarray(string md) : mdName(md)
{
    DIR *dir = NULL;
    struct dirent *ent = NULL;
    uint32_t offset = 0;
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
            if (parseFile(Path, buffer) < 0) {
                continue;
            }
            sscanf(buffer, "%lu", &offset);
            this->Disks.push_back(Disk(ent->d_name, offset));
        }
        closedir (dir);
    }
    else {
        perror("Open slaves directory, failed\n");
    }

    memset(Path, 0, 128);
    memset(buffer, 0, 32);
    sprintf(Path, SYSFS_MD_CHUNK_SIZE_FORMAT, md.c_str());
    if (parseFile(Path, buffer) < 0) {
        goto fail;
    }
    sscanf(buffer, "%lu", &this->chunkSize);

    memset(Path, 0, 128);
    memset(buffer, 0, 32);
    sprintf(Path, SYSFS_MD_LEVEL_FORMAT, md.c_str());
    if (parseFile(Path, buffer) < 0) {
        goto fail;
    }
    this->level = buffer;

fail:
    return;
}

void Diskarray::PrintDiskarray()
{
    vector<Disk>::iterator itrDisk;
    printf("md name: %s\n", this->mdName.c_str());
    printf("chunk size: %lu\n", this->chunkSize);
    printf("level: %s\n", this->level.c_str());

    printf("Disks:\n");
    for (itrDisk = this->Disks.begin(); itrDisk != this->Disks.end(); itrDisk++) {
        printf("\t");
        itrDisk->PrintDisk();
    }
}

Disk::Disk(string name, uint32_t offset) : name(name), offset(offset)
{}

void Disk::PrintDisk()
{
    printf("Disk name: %s, Offset: %lu\n", this->name.c_str(), this->offset);
}
