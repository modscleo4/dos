#!/bin/python3

import sys

def virt2phys(addr: int) -> tuple[int, int, int]:
    pd_index = (addr >> 22) & 0x3ff
    pt_index = (addr >> 12) & 0x3ff
    offset = addr & 0xfff
    return (pd_index, pt_index, offset)


def main(args: list[str]) -> int:
    if len(args) == 1:
        addr = int(args[0], 0)
        print(virt2phys(addr))
    else:
        print("Usage: virt2phys.py [addr]")
        return 1

    return 0

if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
