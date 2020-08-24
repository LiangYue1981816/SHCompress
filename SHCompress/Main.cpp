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

int GetDataCount(const char* szDataFileName)
{
	int lines = 0;

	if (FILE *pFile = fopen(szDataFileName, "rb")) {
		while (!feof(pFile)) {
			float data[9] = { 0.0f };

			for (int j = 0; j < 9; j++) {
				fscanf(pFile, "%f", &data[j]);
			}

			lines++;
		}
		fclose(pFile);
	}

	return lines;
}

void Test12(const char* szDataFileName, int d)
{
	const int n = 3 * 3;
	const int count = GetDataCount(szDataFileName) / 3;
	float **data_set = (float **)AllocMatrix(n, count, sizeof(float));

	if (FILE *pFile = fopen(szDataFileName, "rb")) {
		for (int index = 0; index < count; index++) {
			for (int i = 0; i < 3; i++) {
				float data[9] = { 0.0f };

				for (int j = 0; j < 9; j++) {
					fscanf(pFile, "%f", &data[j]);
				}

				for (int j = 0; j < 3; j++) {
					data_set[i * 3 + j][index] = data[j + 1] / PI;
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
	SHAlloc2(&sh_data, d);
	SHBuild(&sh_data, data_set, count);

	IMAGE imgSource;
	IMAGE imgCompress;
	IMAGE imgPreview;
	IMAGE_ZeroImage(&imgSource);
	IMAGE_ZeroImage(&imgCompress);
	IMAGE_ZeroImage(&imgPreview);
	IMAGE_AllocImage(&imgSource, size * 4, size * 3, 24);
	IMAGE_AllocImage(&imgCompress, size * 4, size * 3, 24);
	IMAGE_AllocImage(&imgPreview, size * 4, size * 3 * 2, 24);

	if (FILE *pFile = fopen(szDataFileName, "rb")) {
		for (int index = 0; index < count; index++) {
			float sh_0[12] = { 0.0f };
			float sh_1[12] = { 0.0f };
			float compress[12] = { 0.0f };

			for (int i = 0; i < 3; i++) {
				float data[9] = { 0.0f };

				for (int j = 0; j < 9; j++) {
					fscanf(pFile, "%f", &data[j]);
				}

				for (int j = 0; j < 4; j++) {
					sh_0[i * 4 + j] = data[j] / PI;
				}
			}

			SHCompress2(&sh_data, sh_0, compress);
			SHUncompress2(&sh_data, compress, sh_1);

			printf("Test12 %d/%d\n", index, count);

			char szFileName[260];
			sprintf(szFileName, "./Result2/%d_%d.jpg", index, d);

			Preview4(&imgSource, &sh_0[0], &sh_0[4], &sh_0[8], size);
			IMAGE_SetImageArea(&imgPreview, 0, 0, IMAGE_WIDTH(&imgPreview) - 1, IMAGE_HEIGHT(&imgPreview) / 2 - 1);
			IMAGE_CopyImageArea(&imgSource, &imgPreview);

			Preview4(&imgCompress, &sh_1[0], &sh_1[4], &sh_1[8], size);
			IMAGE_SetImageArea(&imgPreview, 0, IMAGE_HEIGHT(&imgPreview) / 2, IMAGE_WIDTH(&imgPreview) - 1, IMAGE_HEIGHT(&imgPreview) - 1);
			IMAGE_CopyImageArea(&imgCompress, &imgPreview);

			IMAGE_SaveJpg(szFileName, &imgPreview, 75);
		}
		fclose(pFile);
	}

	IMAGE_FreeImage(&imgSource);
	IMAGE_FreeImage(&imgCompress);
	IMAGE_FreeImage(&imgPreview);

	SHFree(&sh_data);
	FreeMatrix((void **)data_set);

	return;
}

void Test27(const char* szDataFileName, int d)
{
	const int n = 8 * 3;
	const int count = GetDataCount(szDataFileName) / 3;
	float **data_set = (float **)AllocMatrix(n, count, sizeof(float));

	if (FILE *pFile = fopen(szDataFileName, "rb")) {
		for (int index = 0; index < count; index++) {
			for (int i = 0; i < 3; i++) {
				float data[9] = { 0.0f };

				for (int j = 0; j < 9; j++) {
					fscanf(pFile, "%f", &data[j]);
				}

				for (int j = 0; j < 8; j++) {
					data_set[i * 8 + j][index] = data[j + 1] / PI;
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
	SHAlloc3(&sh_data, d);
	SHBuild(&sh_data, data_set, count);
	
	IMAGE imgSource;
	IMAGE imgCompress;
	IMAGE imgPreview;
	IMAGE_ZeroImage(&imgSource);
	IMAGE_ZeroImage(&imgCompress);
	IMAGE_ZeroImage(&imgPreview);
	IMAGE_AllocImage(&imgSource, size * 4, size * 3, 24);
	IMAGE_AllocImage(&imgCompress, size * 4, size * 3, 24);
	IMAGE_AllocImage(&imgPreview, size * 4, size * 3 * 2, 24);

	if (FILE *pFile = fopen(szDataFileName, "rb")) {
		for (int index = 0; index < count; index++) {
			float sh_0[27] = { 0.0f };
			float sh_1[27] = { 0.0f };
			float compress[27] = { 0.0f };

			for (int i = 0; i < 3; i++) {
				float data[9] = { 0.0f };

				for (int j = 0; j < 9; j++) {
					fscanf(pFile, "%f", &data[j]);
				}

				for (int j = 0; j < 9; j++) {
					sh_0[i * 9 + j] = data[j] / PI;
				}
			}

			SHCompress3(&sh_data, sh_0, compress);
			SHUncompress3(&sh_data, compress, sh_1);

			printf("Test27 %d/%d\n", index, count);

			char szFileName[260];
			sprintf(szFileName, "./Result3/%d_%d.jpg", index, d);

			Preview9(&imgSource, &sh_0[0], &sh_0[9], &sh_0[18], size);
			IMAGE_SetImageArea(&imgPreview, 0, 0, IMAGE_WIDTH(&imgPreview) - 1, IMAGE_HEIGHT(&imgPreview) / 2 - 1);
			IMAGE_CopyImageArea(&imgSource, &imgPreview);

			Preview9(&imgCompress, &sh_1[0], &sh_1[9], &sh_1[18], size);
			IMAGE_SetImageArea(&imgPreview, 0, IMAGE_HEIGHT(&imgPreview) / 2, IMAGE_WIDTH(&imgPreview) - 1, IMAGE_HEIGHT(&imgPreview) - 1);
			IMAGE_CopyImageArea(&imgCompress, &imgPreview);

			IMAGE_SaveJpg(szFileName, &imgPreview, 75);
		}
		fclose(pFile);
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

	Test12("./data/data_rgb.txt", 3);
	Test27("./data/data_rgb.txt", 4);

	return 0;
}
