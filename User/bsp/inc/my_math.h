#ifndef _H_MATH1_H
#define _H_MATH1_H

//#include "stm32f10x_lib.h"
//#include "soft.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define Length_typ int16_t
#define Length_long int32_t
#define Angle_typ float
#define s32 signed int
#define uint32 unsigned int
#define uint16 unsigned short int
#define s16 signed short int
#define SIN_COS_LEFT_SHIFT_NUM        (14)

#define YX_COS_S32(p, angle)     (((s32)(p)*yx_cos_f32((float)angle))>>SIN_COS_LEFT_SHIFT_NUM)
#define YX_SIN_S32(p, angle)     (((s32)(p)*yx_sin_f32((float)angle))>>SIN_COS_LEFT_SHIFT_NUM)


typedef Length_typ vec_t[2];
typedef struct
{
	vec_t o;
	vec_t p;
}vec2;

extern s32 yx_cos_f32(Angle_typ x);
extern s32 yx_sin_f32(Angle_typ x);
extern Angle_typ yx_atan2_f32(float _x,float _y);
extern float yx_fabs(float _f);
extern float yx_sqrt(float _number);



#endif

