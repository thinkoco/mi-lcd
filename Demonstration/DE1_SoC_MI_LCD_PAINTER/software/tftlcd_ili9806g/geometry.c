#include "terasic_includes.h"
#include "geometry.h"

//#define BOUND(x, mini, maxi) ((x<(mini)?(mini):( (x>(maxi))?(maxi):(x) )))

//#define COPY_POINT(pt_des, pt_src)  memcpy(&pt_des, &pt_src,sizeof(POINT))

bool IsPtInRect(POINT *pt, RECT *rc){
    bool bYes = FALSE;
    
    if (pt->x >= rc->left && pt->x <= rc->right && pt->y >= rc->top && pt->y <= rc->bottom)
        bYes = TRUE;
    
    return bYes;
}

void PtSet(POINT *pt, int x, int y){
    pt->x = x;
    pt->y = y;
}

void RectSet(RECT *rc, int left, int right, int top, int bottom){
    rc->left = left;
    rc->right = right;
    rc->top = top;
    rc->bottom = bottom;
}

void RectInflate(RECT *rc, int x, int y){
    rc->left -= x;
    rc->right += x;
    rc->top -= y;
    rc->bottom += y;
}

void RectOffset(RECT *rc, int x, int y){
    rc->left += x;
    rc->right += x;
    rc->top += y;
    rc->bottom += y;
}

void RectCopy(RECT *rcDes, RECT *rcSrc){
    rcDes->left = rcSrc->left;
    rcDes->right = rcSrc->right;
    rcDes->top = rcSrc->top;
    rcDes->bottom = rcSrc->bottom;
}

int RectWidth(RECT *rc){
    return (rc->right-rc->left);
}

int RectHeight(RECT *rc){
    return (rc->bottom-rc->top);
}

int PtDistance(POINT *pt1, POINT *pt2){
    int nDistance;
    int a, b;
    a = pt1->x - pt2->x;
    b = pt1->y - pt2->y;
    nDistance = a*a + b*b;
    nDistance = sqrt(nDistance);
    
    return nDistance;
}

void PtCopy(POINT *ptDes, POINT *ptSrc){
    ptDes->x = ptSrc->x;
    ptDes->y = ptSrc->y;
}

