#pragma once
#include <stdio.h>
#include <stdlib.h>


typedef struct SHData {
	int N;
	int D;
	float *mean;
	float *eigval;
	float **eigvec;
} SHData;


extern void** AllocMatrix(int nrows, int ncols, int nsize);
extern void FreeMatrix(void **pmatrix);

extern void SHInit(SHData *sh_data);
extern void SHAlloc2(SHData *sh_data, int d);
extern void SHAlloc3(SHData *sh_data, int d);
extern void SHFree(SHData *sh_data);

extern int SHBuild(SHData *sh_data, float **data_set, int count);

extern void SHCompress(SHData *sh_data, float *source_data, float *compress_data);
extern void SHUncompress(SHData *sh_data, float *compress_data, float *source_data);
