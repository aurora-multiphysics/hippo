#!/usr/bin/env bash

awk -v var=$RANDOM 'function rint(n,m) {
    return int(n + (m - n)*rand())
}
BEGIN {
    srand(var)
    printf("%d %d %d\n%d %d %d\n%d %d %d %d %d %d",
            rint(1,21), rint(1,21), rint(1,21),
            rint(1,3), rint(1,3), rint(1,3),
            rint(0,2), rint(0,2), rint(0,2),
            rint(0,2), rint(0,2), rint(0,2));
} ' > $1
