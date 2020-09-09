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
    this->diskarray.PrintDiskarray();
}

void Mapper::printReverseMapping(void)
{
}

int main(int argc, char *argv[])
{
    string operation;
    string mdName, diskName;
    int sector = -1;

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
        sscanf(argv[3], "%d", &sector);
        Mapper mapper = Mapper(operation, mdName, "", sector);
        mapper.printMapping();
    }
    else if (operation == "r") {
        mdName = argv[2];
        diskName = argv[3];
        sscanf(argv[4], "%d", &sector);
        Mapper mapper = Mapper(operation, mdName, diskName, sector);
        mapper.printReverseMapping();
    }
    else {
        printf("Invalid option\n");
        goto fail;
    }

end:
    return 0;
fail:
    return -1;
}
