#ifndef POINT_H
#define POINT_H

#include <stdio.h>
#include <stdlib.h>
typedef struct point1 {
    int x;
    int y;
} Point;

Point* setPoint( int x, int y) {
    Point* p = (Point*)malloc(sizeof(Point));
    p->x = x;
    p->y = y;
    return p;
}

#endif