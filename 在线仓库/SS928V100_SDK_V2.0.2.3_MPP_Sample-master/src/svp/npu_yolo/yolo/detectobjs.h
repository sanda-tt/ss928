#ifndef __DETECTOBJS_H__
#define __DETECTOBJS_H__

#ifdef __cplusplus
extern "C" {
#endif

#define OBJDETECTMAX 256
typedef struct {
  int x;
  int y;
  int w;
  int h;
  int label;
  float score;
} stObjinfo;

typedef struct {
  int count;
  stObjinfo objs[OBJDETECTMAX];
} stYolov5Objs;

#ifdef __cplusplus
}
#endif
#endif
