#!/bin/python3

import sys
from typing import List

HEADS = 2
CYLINDERS = 80
SECTORS_PER_TRACK = 18
SECTOR_SIZE = 512


def lba2chs(lba: int) -> tuple[int, int, int]:
    cylinders = lba // (HEADS * SECTORS_PER_TRACK)
    head      = (lba // SECTORS_PER_TRACK) % HEADS
    sector    = (lba % SECTORS_PER_TRACK) + 1
    return (cylinders, head, sector)


def chs2lba(cylinder: int, head: int, sector: int) -> int:
    return (cylinder * HEADS + head) * SECTORS_PER_TRACK + (sector - 1)


def main(args: List[str]) -> int:
    if len(args) == 1:
        lba = int(args[0])
        print(lba2chs(lba))
    elif len(args) == 3:
        cylinder = int(args[0])
        head     = int(args[1])
        sector   = int(args[2])
        lba      = chs2lba(cylinder, head, sector)
        print(lba)
    else:
        print("Usage: lba2chs.py [lba] | lba2chs.py [cylinder] [head] [sector]")
        return 1

    return 0

if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
