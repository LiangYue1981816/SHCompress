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
extern void SHAlloc(SHData *sh_data, int n);
extern void SHFree(SHData *sh_data);

extern int SHBuild(SHData *sh_data, float **data_set, int count, float percent);
extern int SHCompress(SHData *sh_data, float *uncompress_data, float *compress_data);
extern int SHUncompress(SHData *sh_data, float *compress_data, float *uncompress_data);
