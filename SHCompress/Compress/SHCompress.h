#pragma once
#include <stdio.h>
#include <stdlib.h>


typedef struct SHData {
	int n;
	float *mean;
	float *eigval;
	float **eigvec;
} SHData;


extern void** AllocMatrix(int nrows, int ncols, int nsize);
extern void FreeMatrix(void **pmatrix);

extern void SHInit(SHData *sh_data);
extern void SHAlloc2(SHData *sh_data);
extern void SHAlloc3(SHData *sh_data);
extern void SHFree(SHData *sh_data);

extern void SHBuild2(SHData *sh_data, float **data_set, int count);
extern void SHBuild3(SHData *sh_data, float **data_set, int count);

extern void SHCompress2(SHData *sh_data, float *source_data, float *compress_data, int d);
extern void SHUncompress2(SHData *sh_data, float *compress_data, float *source_data, int d);

extern void SHCompress3(SHData *sh_data, float *source_data, float *compress_data, int d);
extern void SHUncompress3(SHData *sh_data, float *compress_data, float *source_data, int d);
