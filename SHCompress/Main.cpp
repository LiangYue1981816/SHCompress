#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

#include "gl/glew.h"
#include "gl/glut.h"

#include "Image.h"
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

static int size = 64;
extern void Preview4(IMAGE *pImage, float *sh_red, float *sh_grn, float *sh_blu, int size);
extern void Preview9(IMAGE *pImage, float *sh_red, float *sh_grn, float *sh_blu, int size);

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

	IMAGE imgSource;
	IMAGE imgCompress;
	IMAGE imgPreview;
	IMAGE_ZeroImage(&imgSource);
	IMAGE_ZeroImage(&imgCompress);
	IMAGE_ZeroImage(&imgPreview);
	IMAGE_AllocImage(&imgSource, size * 4, size * 3, 24);
	IMAGE_AllocImage(&imgCompress, size * 4, size * 3, 24);
	IMAGE_AllocImage(&imgPreview, size * 4, size * 3 * 2, 24);

	for (int index = 0; index < count; index++) {
		char szFileName[260];

		float sh_0[n] = { 0.0f };
		float sh_1[n] = { 0.0f };
		float compress[n] = { 0.0f };

		for (int i = 0; i < n; i++) {
			sh_0[i] = data_set[i][index];
		}

		printf("Test12 %d/%d\n", index, count);

		Preview4(&imgSource, &sh_0[0], &sh_0[4], &sh_0[8], size);

		SHCompress(&sh_data, sh_0, compress);
		SHUncompress(&sh_data, compress, sh_1);

		Preview4(&imgCompress, &sh_1[0], &sh_1[4], &sh_1[8], size);

		IMAGE_SetImageArea(&imgPreview, 0, 0, IMAGE_WIDTH(&imgPreview) - 1, IMAGE_HEIGHT(&imgPreview) / 2 - 1);
		IMAGE_CopyImageArea(&imgSource, &imgPreview);

		IMAGE_SetImageArea(&imgPreview, 0, IMAGE_HEIGHT(&imgPreview) / 2, IMAGE_WIDTH(&imgPreview) - 1, IMAGE_HEIGHT(&imgPreview) - 1);
		IMAGE_CopyImageArea(&imgCompress, &imgPreview);

		sprintf(szFileName, "./Result/%d_2.jpg", index);
		IMAGE_SaveJpg(szFileName, &imgPreview, 75);
	}

	IMAGE_FreeImage(&imgSource);
	IMAGE_FreeImage(&imgCompress);
	IMAGE_FreeImage(&imgPreview);

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

	IMAGE imgSource;
	IMAGE imgCompress;
	IMAGE imgPreview;
	IMAGE_ZeroImage(&imgSource);
	IMAGE_ZeroImage(&imgCompress);
	IMAGE_ZeroImage(&imgPreview);
	IMAGE_AllocImage(&imgSource, size * 4, size * 3, 24);
	IMAGE_AllocImage(&imgCompress, size * 4, size * 3, 24);
	IMAGE_AllocImage(&imgPreview, size * 4, size * 3 * 2, 24);

	for (int index = 0; index < count; index++) {
		char szFileName[260];

		float sh_0[n] = { 0.0f };
		float sh_1[n] = { 0.0f };
		float compress[n] = { 0.0f };

		for (int i = 0; i < n; i++) {
			sh_0[i] = data_set[i][index];
		}

		printf("Test12 %d/%d\n", index, count);

		Preview9(&imgSource, &sh_0[0], &sh_0[9], &sh_0[18], size);

		SHCompress(&sh_data, sh_0, compress);
		SHUncompress(&sh_data, compress, sh_1);

		Preview9(&imgCompress, &sh_1[0], &sh_1[9], &sh_1[18], size);

		IMAGE_SetImageArea(&imgPreview, 0, 0, IMAGE_WIDTH(&imgPreview) - 1, IMAGE_HEIGHT(&imgPreview) / 2 - 1);
		IMAGE_CopyImageArea(&imgSource, &imgPreview);

		IMAGE_SetImageArea(&imgPreview, 0, IMAGE_HEIGHT(&imgPreview) / 2, IMAGE_WIDTH(&imgPreview) - 1, IMAGE_HEIGHT(&imgPreview) - 1);
		IMAGE_CopyImageArea(&imgCompress, &imgPreview);

		sprintf(szFileName, "./Result/%d_3.jpg", index);
		IMAGE_SaveJpg(szFileName, &imgPreview, 75);
	}

	IMAGE_FreeImage(&imgSource);
	IMAGE_FreeImage(&imgCompress);
	IMAGE_FreeImage(&imgPreview);

	SHFree(&sh_data);
	FreeMatrix((void **)data_set);

	return;
}

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);

	glutInitWindowSize(0, 0);
	glutInitWindowPosition(0, 0);
	glutCreateWindow("");
	glewInit();

	Test12(0.9f);
	Test27(0.9f);

	return 0;
}
