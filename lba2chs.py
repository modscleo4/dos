#!/bin/python3

import sys

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


def main(args: list[str]) -> int:
    if len(args) != 2 and len(args) != 4:
        print(f"Usage: {args[0]} [lba] | {args[0]} [cylinder] [head] [sector]")
        return 1

    if len(args) == 2:
        lba = int(args[1])
        print(lba2chs(lba))
    else:
        cylinder = int(args[1])
        head     = int(args[2])
        sector   = int(args[3])
        lba      = chs2lba(cylinder, head, sector)
        print(lba)

    return 0


if __name__ == '__main__':
    sys.exit(main(sys.argv))
