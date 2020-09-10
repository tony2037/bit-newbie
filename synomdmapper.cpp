/* Copyright (c) Synology Inc. All rights reserved.*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "synomdmapper.h"

using namespace std;

Mapper::Mapper(string ops, string md, string disk, int qsector) : operation(ops), mdName(md), diskName(disk),
    querySector(qsector), diskarray(Diskarray(md))
{
}

void Mapper::printMapping(void)
{
    int disk = -1;
    uint32_t diskSector = 0;
    this->diskarray.PrintDiskarray();
    disk = this->diskarray.GetDiskSector(this->querySector, &diskSector);
    printf("(sector)[%s]:%lu = [%s]:%lu\n",
            this->mdName.c_str(), this->querySector, this->diskarray.Disks[disk].name.c_str(), diskSector);
}

void Mapper::printReverseMapping(void)
{
    uint64_t raidSector = 0;
    this->diskarray.PrintDiskarray();
    raidSector = this->diskarray.GetRaidSector(this->diskName, this->querySector);
    if (raidSector == UINT64_MAX) {
        printf("Check the credity\n");
    }
    printf("(sector)[%s]:%lu = [%s]:%lu\n",
            this->mdName.c_str(), raidSector, this->diskName.c_str(), this->querySector);
}

int main(int argc, char *argv[])
{
    string operation;
    string mdName;
    string diskName;
    uint32_t sector = 0;

    if (argc < 4) {
        printf("Usage:\n");
        printf("    synomdmapper [m/r] sector\n");
        printf("Example:\n");
        printf("Get mapping, md start sector 0: synomdmapper m md3 0\n");
        printf("Get reverse mapping, disk 3 start sector 2176: synomdmapper r md3 sata3 2176\n");
        goto fail;
    }

    operation = argv[1];
    if (operation == "m") {
        mdName = argv[2];
        sscanf(argv[3], "%lu", &sector);
        Mapper mapper = Mapper(operation, mdName, "", sector);
        mapper.printMapping();
    }
    else if (operation == "r") {
        mdName = argv[2];
        sscanf(argv[4], "%lu", &sector);
        Mapper mapper = Mapper(operation, mdName, argv[3], sector);
        mapper.printReverseMapping();
    }
    else {
        printf("Invalid option\n");
        goto fail;
    }

end:
    exit(0);
fail:
    exit(-1);
}
