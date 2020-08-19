#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

#include "gl/glew.h"
#include "gl/glut.h"

#include "SHCompress.h"

#define PI 3.1415926535897932384626433832795f

#ifdef __cplusplus
extern "C" {
#endif

extern void** allocmatrix(int nrows, int ncols, int nsize);
extern void freematrix(void **pmatrix);

#ifdef __cplusplus
}
#endif

extern void Preview4(const char *szFileName, float *sh_red, float *sh_grn, float *sh_blu);
extern void Preview9(const char *szFileName, float *sh_red, float *sh_grn, float *sh_blu);

void Test12(float percent)
{
	const int n = 4 * 3;
	const int count = 3741 / 3;
	float **data_set = (float **)AllocMatrix(n, count, sizeof(float));

	if (FILE *pFile = fopen("data.txt", "rb")) {
		for (int index = 0; index < count; index++) {
			for (int i = 0; i < 3; i++) {
				float data[9] = { 0.0f };

				for (int j = 0; j < 9; j++) {
					fscanf(pFile, "%f", &data[j]);
				}

				for (int j = 0; j < n / 3; j++) {
					data_set[i * n / 3 + j][index] = data[j] / PI;
				}
			}
		}
		fclose(pFile);
	}
	else {
		FreeMatrix((void **)data_set);
		return;
	}

	SHData sh_data;
	SHInit(&sh_data);
	SHAlloc(&sh_data, n);
	SHBuild(&sh_data, data_set, count, percent);

	for (int index = 0; index < count; index++) {
		char szFileName[260];

		float sh_0[n] = { 0.0f };
		float sh_1[n] = { 0.0f };
		float compress[n] = { 0.0f };

		for (int i = 0; i < n; i++) {
			sh_0[i] = data_set[i][index];
		}

		printf("Test12 %d/%d\n", index, count);

		sprintf(szFileName, "./Result/%4.4d_0.jpg", index);
		Preview4(szFileName, &sh_0[0], &sh_0[4], &sh_0[8]);

		SHCompress(&sh_data, sh_0, compress);
		SHUncompress(&sh_data, compress, sh_1);

		sprintf(szFileName, "./Result/%4.4d_12.jpg", index);
		Preview4(szFileName, &sh_1[0], &sh_1[4], &sh_1[8]);
	}

	SHFree(&sh_data);
	FreeMatrix((void **)data_set);
	return;
}

void Test27(float percent)
{
	const int n = 9 * 3;
	const int count = 3741 / 3;
	float **data_set = (float **)AllocMatrix(n, count, sizeof(float));

	if (FILE *pFile = fopen("data.txt", "rb")) {
		for (int index = 0; index < count; index++) {
			for (int i = 0; i < 3; i++) {
				float data[9] = { 0.0f };

				for (int j = 0; j < 9; j++) {
					fscanf(pFile, "%f", &data[j]);
				}

				for (int j = 0; j < n / 3; j++) {
					data_set[i * n / 3 + j][index] = data[j] / PI;
				}
			}
		}
		fclose(pFile);
	}
	else {
		FreeMatrix((void **)data_set);
		return;
	}

	SHData sh_data;
	SHInit(&sh_data);
	SHAlloc(&sh_data, n);
	SHBuild(&sh_data, data_set, count, percent);

	for (int index = 0; index < count; index++) {
		char szFileName[260];

		float sh_0[n] = { 0.0f };
		float sh_1[n] = { 0.0f };
		float compress[n] = { 0.0f };

		for (int i = 0; i < n; i++) {
			sh_0[i] = data_set[i][index];
		}

		printf("Test12 %d/%d\n", index, count);

		sprintf(szFileName, "./Result/%4.4d_0.jpg", index);
		Preview9(szFileName, &sh_0[0], &sh_0[9], &sh_0[18]);

		SHCompress(&sh_data, sh_0, compress);
		SHUncompress(&sh_data, compress, sh_1);

		sprintf(szFileName, "./Result/%4.4d_27.jpg", index);
		Preview9(szFileName, &sh_1[0], &sh_1[9], &sh_1[18]);
	}

	SHFree(&sh_data);
	FreeMatrix((void **)data_set);
	return;

	/*
	const int n = 9 * 3;
	const int count = 3741 / 3;
	float **sh_data = (float **)allocmatrix(n, count, sizeof(float));

	if (FILE *pFile = fopen("data9.txt", "rb")) {
		for (int index = 0; index < count; index++) {
			for (int i = 0; i < n; i++) {
				fscanf(pFile, "%f", &sh_data[i][index]);
				sh_data[i][index] /= PI;
			}
		}
		fclose(pFile);
	}
	else {
		freematrix((void **)sh_data);
		return;
	}

	PCAMODEL model;
	PCA_InitModel(&model);
	PCA_AllocModel(n, &model);
	PCA_DataPCA(sh_data, n, count, percent, model.mean, model.eigval, model.eigvec, &model.D);
//	PCA_DataPCAEx(sh_data, n, count, model.mean, model.eigval, model.eigvec, 4); model.D = 4;

	for (int index = 0; index < count; index++) {
		char szFileName[260];

		float sh_0[n] = { 0.0f };
		float sh_1[n] = { 0.0f };
		float compress[n] = { 0.0f };

		for (int i = 0; i < n; i++) {
			sh_0[i] = sh_data[i][index];
		}

		printf("Test27 %d/%d\n", index, count);

		sprintf(szFileName, "./Result/%4.4d_0.jpg", index);
		Preview9(szFileName, &sh_0[0], &sh_0[9], &sh_0[18]);

		PCA_DataParam(sh_0, model.mean, model.eigvec, model.N, model.D, compress);
		PCA_DataInvPCA(model.mean, model.eigvec, compress, model.N, model.D, sh_1);

		sprintf(szFileName, "./Result/%4.4d_27.jpg", index);
		Preview9(szFileName, &sh_1[0], &sh_1[9], &sh_1[18]);
	}

	PCA_FreeModel(&model);
	freematrix((void **)sh_data);
	return;
	*/
}

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);

	glutInitWindowSize(0, 0);
	glutInitWindowPosition(0, 0);
	glutCreateWindow("");
	glewInit();

//	Test12(0.9f);
	Test27(0.9f);

	return 0;
}
