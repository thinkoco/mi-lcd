#ifndef GEOMETRY_H_
#define GEOMETRY_H_

typedef struct{
    int left;
    int right;
    int top;
    int bottom;
}RECT;

typedef struct{
    int x;
    int y;
}POINT;

bool IsPtInRect(POINT *pt, RECT *rc);
void PtSet(POINT *pt, int x, int y);
void RectSet(RECT *rc, int left, int right, int top, int bottom);
void RectInflate(RECT *rc, int x, int y);
void RectOffset(RECT *rc, int x, int y);
void RectCopy(RECT *rcDes, RECT *rcSrc);
int RectWidth(RECT *rc);
int RectHeight(RECT *rc);
int PtDistance(POINT *pt1, POINT *pt2);
void PtCopy(POINT *ptDes, POINT *ptSrc);


#endif /*GEOMETRY_H_*/
