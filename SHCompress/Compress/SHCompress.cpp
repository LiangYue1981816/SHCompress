#include <math.h>
#include "SHCompress.h"


static float* alloc_vector(int nl, int nh)
{
	float *v = NULL;

	v = (float*)calloc(nh - nl + 2, sizeof(float));
	if (NULL == v) return NULL;

	return v - nl + 1;
}

static void free_vector(float *v, int nl, int nh)
{
	if (v + nl - 1) {
		free(v + nl - 1);
	}
}

static void** alloc_matrix(int nrows, int ncols, int nsize)
{
	int i, nheadersize, nbuffersize, nlinesize;
	unsigned char *pbuffer = NULL, **ppline = NULL, *pbody = NULL;

	if (nrows <= 0 || ncols <= 0 || nsize <= 0)
		return NULL;

	nlinesize = ((ncols*nsize + 3) / 4) * 4;
	nheadersize = ((nrows * sizeof(void *) + 3) / 4) * 4;
	nbuffersize = nheadersize + nlinesize * nrows;

	pbuffer = (unsigned char *)calloc(nbuffersize, sizeof(unsigned char));
	if (NULL == pbuffer) return NULL;

	ppline = (unsigned char **)pbuffer;
	pbody = pbuffer + nbuffersize - nlinesize;

	for (i = 0; i < nrows; i++) {
		*ppline = pbody;
		pbody -= nlinesize;
		ppline++;
	}

	return (void **)pbuffer;
}

static void free_matrix(void **pmatrix)
{
	if (pmatrix) {
		free(pmatrix);
	}
}

static void convert_matrix(void ***pmatrix, int nrows, int ncols, int nsize)
{
	int i, nheadersize, nbuffersize, nlinesize;
	unsigned char *pbuffer = NULL, **ppline = NULL, *pbody = NULL;

	if (nrows <= 0 || ncols <= 0 || nsize <= 0)
		return;

	nlinesize = ((ncols*nsize + 3) / 4) * 4;
	nheadersize = ((nrows * sizeof(void *) + 3) / 4) * 4;
	nbuffersize = nheadersize + nlinesize * nrows;

	pbuffer = (unsigned char *)(*pmatrix);
	ppline = (unsigned char **)(pbuffer);
	pbody = pbuffer + nbuffersize - nlinesize - nsize;

	for (i = 0; i < nrows; i++) {
		*ppline = pbody;
		pbody -= nlinesize;
		ppline++;
	}

	(*pmatrix)--;

	return;
}

static void unconvert_matrix(void ***pmatrix, int nrows, int ncols, int nsize)
{
	int i, nheadersize, nbuffersize, nlinesize;
	unsigned char *pbuffer = NULL, **ppline = NULL, *pbody = NULL;

	if (nrows <= 0 || ncols <= 0 || nsize <= 0)
		return;

	nlinesize = ((ncols*nsize + 3) / 4) * 4;
	nheadersize = ((nrows * sizeof(void *) + 3) / 4) * 4;
	nbuffersize = nheadersize + nlinesize * nrows;

	(*pmatrix)++;

	pbuffer = (unsigned char *)(*pmatrix);
	ppline = (unsigned char **)(pbuffer);
	pbody = pbuffer + nbuffersize - nlinesize;

	for (i = 0; i < nrows; i++) {
		*ppline = pbody;
		pbody -= nlinesize;
		ppline++;
	}

	return;
}

static void convert_vector(void **pvector, int nsize)
{
	unsigned char **pvoid = NULL;

	if (NULL == pvector || nsize <= 0)
		return;

	pvoid = (unsigned char**)pvector;
	(*pvoid) -= nsize;
}

static void unconvert_vector(void **pvector, int nsize)
{
	unsigned char **pvoid = NULL;

	if (NULL == pvector || nsize <= 0)
		return;

	pvoid = (unsigned char**)pvector;
	(*pvoid) += nsize;
}

static void matrix_mul_matrix(float **a, float **b, float **c, int rows, int cols_rows, int cols)
{
	int i, j, k;

	for (i = 0; i < rows; i++) {
		for (j = 0; j < cols; j++) {
			c[i][j] = 0.0f;
			for (k = 0; k < cols_rows; k++) {
				c[i][j] += a[i][k] * b[k][j];
			}
		}
	}
}

static void matrix_mul_matrix_transpose(float **a, float **b, float **c, int rows, int cols)
{
	int i, j, k;

	for (i = 0; i < rows; i++) {
		for (j = 0; j < rows; j++) {
			c[i][j] = 0.0f;
			for (k = 0; k < cols; k++) {
				c[i][j] += a[i][k] * b[j][k];
			}
		}
	}
}

static void matrix_transpose_mul_matrix(float **a, float **b, float **c, int rows, int cols)
{
	int i, j, k;

	for (i = 0; i < cols; i++) {
		for (j = 0; j < cols; j++) {
			c[i][j] = 0.0f;
			for (k = 0; k < rows; k++) {
				c[i][j] += a[k][i] * b[k][j];
			}
		}
	}
}

static void jacobi(float **a, int n, float d[], float **v, int *nrot)
{
	#define ROTATE(a,i,j,k,l) do { g = a[i][j]; h = a[k][l]; a[i][j] = g - s * (h + g * tau); a[k][l] = h + s * (g - h * tau); } while(false);

	int j, iq, ip, i;
	float tresh, theta, tau, t, sm, s, h, g, c, *b, *z;

	b = alloc_vector(1, n);
	z = alloc_vector(1, n);
	for (ip = 1; ip <= n; ip++) {
		for (iq = 1; iq <= n; iq++) {
			v[ip][iq] = 0.0f;
		}
		v[ip][ip] = 1.0f;
	}
	for (ip = 1; ip <= n; ip++) {
		b[ip] = d[ip] = a[ip][ip];
		z[ip] = 0.0f;
	}
	*nrot = 0;
	for (i = 1; i <= 50; i++) {
		sm = 0.0f;
		for (ip = 1; ip <= n - 1; ip++) {
			for (iq = ip + 1; iq <= n; iq++) {
				sm += fabsf(a[ip][iq]);
			}
		}
		if (sm == 0.0f) {
			free_vector(z, 1, n);
			free_vector(b, 1, n);
			return;
		}
		if (i < 4) {
			tresh = 0.2f * sm / (n * n);
		}
		else {
			tresh = 0.0f;
		}
		for (ip = 1; ip <= n - 1; ip++) {
			for (iq = ip + 1; iq <= n; iq++) {
				g = 100.0f * fabsf(a[ip][iq]);
				if (i > 4 && fabsf(d[ip]) + g == fabsf(d[ip]) && fabsf(d[iq]) + g == fabsf(d[iq])) {
					a[ip][iq] = 0.0f;
				}
				else if (fabsf(a[ip][iq]) > tresh) {
					h = d[iq] - d[ip];
					if (fabsf(h) + g == fabsf(h)) {
						t = a[ip][iq] / h;
					}
					else {
						theta = 0.5f * h / a[ip][iq];
						t = 1.0f / (fabsf(theta) + sqrtf(1.0f + theta * theta));
						if (theta < 0.0f) {
							t = -t;
						}
					}
					c = 1.0f / sqrtf(1 + t * t);
					s = t * c;
					tau = s / (1.0f + c);
					h = t * a[ip][iq];
					z[ip] -= h;
					z[iq] += h;
					d[ip] -= h;
					d[iq] += h;
					a[ip][iq] = 0.0f;
					for (j = 1; j <= ip - 1; j++) {
						ROTATE(a, j, ip, j, iq)
					}
					for (j = ip + 1; j <= iq - 1; j++) {
						ROTATE(a, ip, j, j, iq)
					}
					for (j = iq + 1; j <= n; j++) {
						ROTATE(a, ip, j, iq, j)
					}
					for (j = 1; j <= n; j++) {
						ROTATE(v, j, ip, j, iq)
					}
					++(*nrot);
				}
			}
		}
		for (ip = 1; ip <= n; ip++) {
			b[ip] += z[ip];
			d[ip] = b[ip];
			z[ip] = 0.0f;
		}
	}
	free_vector(z, 1, n);
	free_vector(b, 1, n);
}

static void eigsrt(float d[], float **v, int n)
{
	int k, j, i;
	float p;

	for (i = 1; i < n; i++) {
		p = d[k = i];
		for (j = i + 1; j <= n; j++) {
			if (d[j] >= p) {
				p = d[k = j];
			}
		}
		if (k != i) {
			d[k] = d[i];
			d[i] = p;
			for (j = 1; j <= n; j++) {
				p = v[j][i];
				v[j][i] = v[j][k];
				v[j][k] = p;
			}
		}
	}
}

static int build(float **mtrx, int rows, int cols, float *mean, float **eigvec, float *eigval, float percent)
{
	int i, j, rot, t = 0;
	float sum, addup;
	float **c = NULL, **cct = NULL;
	float *val = NULL, **vec = NULL;

	c = (float **)alloc_matrix(rows, cols, sizeof(float));
	cct = (float **)alloc_matrix(rows, rows, sizeof(float));
	if (NULL == c || NULL == cct) { goto RET; }

	for (i = 0; i < rows; i++) { for (j = 0; j < cols; j++) { mean[i] += mtrx[i][j]; } }
	for (i = 0; i < rows; i++) { mean[i] /= cols; }
	for (i = 0; i < rows; i++) { for (j = 0; j < cols; j++) { c[i][j] = mtrx[i][j] - mean[i]; } }

	matrix_mul_matrix_transpose(c, c, cct, rows, cols);

	val = eigval;
	vec = eigvec;

	convert_matrix((void***)&cct, rows, rows, sizeof(float));
	convert_vector((void **)&val, sizeof(float));
	convert_matrix((void***)&vec, rows, rows, sizeof(float));

	jacobi(cct, rows, val, vec, &rot);
	eigsrt(val, vec, rows);

	unconvert_matrix((void***)&cct, rows, rows, sizeof(float));
	unconvert_vector((void **)&val, sizeof(float));
	unconvert_matrix((void***)&vec, rows, rows, sizeof(float));

	sum = 0.0f; addup = 0.0f;
	for (j = 0; j < rows; j++) { sum += eigval[j]; }
	for (j = 0; j < rows; j++) { addup += eigval[j]; if (addup / sum > percent) { t = j; break; } }

	for (j = 0; j < t; j++) {
		sum = 0.0f; for (i = 0; i < rows; i++) sum += eigvec[i][j] * eigvec[i][j];
		sum = sqrtf(sum);  for (i = 0; i < rows; i++) eigvec[i][j] /= sum;
	}

	for (j = 0; j < t; j++) { eigval[j] = eigval[j] / (cols - 1); }
	for (; j < rows; j++) { for (i = 0; i < rows; i++) { eigvec[i][j] = 0.0f; } eigval[j] = 0.0f; }

RET:
	if (c) free_matrix((void**)c);
	if (cct) free_matrix((void**)cct);

	return t;
}

static float build(float **mtrx, int rows, int cols, float *mean, float **eigvec, float *eigval, int t)
{
	int i, j, rot;
	float sum, addup, percent = 0.0f;
	float **c = NULL, **cct = NULL;
	float *val = NULL, **vec = NULL;

	c = (float **)alloc_matrix(rows, cols, sizeof(float));
	cct = (float **)alloc_matrix(rows, rows, sizeof(float));
	if (NULL == c || NULL == cct) { goto RET; }

	for (i = 0; i < rows; i++) { for (j = 0; j < cols; j++) { mean[i] += mtrx[i][j]; } }
	for (i = 0; i < rows; i++) { mean[i] /= cols; }
	for (i = 0; i < rows; i++) { for (j = 0; j < cols; j++) { c[i][j] = mtrx[i][j] - mean[i]; } }

	matrix_mul_matrix_transpose(c, c, cct, rows, cols);

	val = eigval;
	vec = eigvec;

	convert_matrix((void***)&cct, rows, rows, sizeof(float));
	convert_vector((void **)&val, sizeof(float));
	convert_matrix((void***)&vec, rows, rows, sizeof(float));

	jacobi(cct, rows, val, vec, &rot);
	eigsrt(val, vec, rows);

	unconvert_matrix((void***)&cct, rows, rows, sizeof(float));
	unconvert_vector((void **)&val, sizeof(float));
	unconvert_matrix((void***)&vec, rows, rows, sizeof(float));

	sum = 0.0f; addup = 0.0f;
	for (j = 0; j < rows; j++) { sum += eigval[j]; }
	for (j = 0; j < t;    j++) { addup += eigval[j]; }
	percent = addup / sum;

	for (j = 0; j < t; j++) {
		sum = 0.0f; for (i = 0; i < rows; i++) sum += eigvec[i][j] * eigvec[i][j];
		sum = sqrtf(sum);  for (i = 0; i < rows; i++) eigvec[i][j] /= sum;
	}

	for (j = 0; j < t; j++) { eigval[j] = eigval[j] / (cols - 1); }
	for (; j < rows; j++) { for (i = 0; i < rows; i++) { eigvec[i][j] = 0.0f; } eigval[j] = 0.0f; }

RET:
	if (c) free_matrix((void**)c);
	if (cct) free_matrix((void**)cct);

	return percent;
}

static int compress(float *data, float *mean, float **eigvec, int N, int DIM, float *param)
{
	int n, d;

	if (NULL == data || NULL == mean || NULL == param || NULL == eigvec) {
		return -1;
	}

	for (n = 0; n < N; n++) {
		data[n] = data[n] - mean[n];
	}

	for (d = 0; d < DIM; d++) {
		param[d] = 0.0f;
		for (n = 0; n < N; n++) {
			param[d] += eigvec[n][d] * data[n];
		}
	}

	return 0;
}

static int uncompress(float *mean, float **eigvec, float *param, int N, int DIM, float *data)
{
	int n, d;

	if (NULL == data || NULL == mean || NULL == param || NULL == eigvec) {
		return -1;
	}

	for (n = 0; n < N; n++) {
		data[n] = 0.0f;
		for (d = 0; d < DIM; d++) {
			data[n] += eigvec[n][d] * param[d];
		}
	}

	for (n = 0; n < N; n++) {
		data[n] += mean[n];
	}

	return 0;
}


void** AllocMatrix(int nrows, int ncols, int nsize)
{
	return alloc_matrix(nrows, ncols, nsize);
}

void FreeMatrix(void **pmatrix)
{
	free_matrix(pmatrix);
}


static const float factors12[12] = {
	1000.0f, 1.0f, 1.0f, 1.0f,
	1000.0f, 1.0f, 1.0f, 1.0f,
	1000.0f, 1.0f, 1.0f, 1.0f
};

static const float factors27[27] = {
	1000.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
	1000.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
	1000.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f
};


void SHInit(SHData *sh_data)
{
	sh_data->N = 0;
	sh_data->mean = NULL;
	sh_data->eigval = NULL;
	sh_data->eigvec = NULL;
}

void SHAlloc2(SHData *sh_data)
{
	sh_data->N = 12;
	sh_data->mean = (float *)calloc(sh_data->N, sizeof(float));
	sh_data->eigval = (float *)calloc(sh_data->N, sizeof(float));
	sh_data->eigvec = (float **)alloc_matrix(sh_data->N, sh_data->N, sizeof(float));
}

void SHAlloc3(SHData *sh_data)
{
	sh_data->N = 27;
	sh_data->mean = (float *)calloc(sh_data->N, sizeof(float));
	sh_data->eigval = (float *)calloc(sh_data->N, sizeof(float));
	sh_data->eigvec = (float **)alloc_matrix(sh_data->N, sh_data->N, sizeof(float));
}

void SHFree(SHData *sh_data)
{
	if (sh_data->mean) {
		free(sh_data->mean);
	}

	if (sh_data->eigval) {
		free(sh_data->eigval);
	}

	if (sh_data->eigvec) {
		free_matrix((void **)sh_data->eigvec);
	}

	SHInit(sh_data);
}

void SHBuild2(SHData *sh_data, float **data_set, int count)
{
	for (int i = 0; i < sh_data->N; i++) {
		for (int j = 0; j < count; j++) {
			data_set[i][j] *= factors12[i];
		}
	}

	build(data_set, sh_data->N, count, sh_data->mean, sh_data->eigvec, sh_data->eigval, sh_data->N);
}

void SHBuild3(SHData *sh_data, float **data_set, int count)
{
	for (int i = 0; i < sh_data->N; i++) {
		for (int j = 0; j < count; j++) {
			data_set[i][j] *= factors27[i];
		}
	}

	build(data_set, sh_data->N, count, sh_data->mean, sh_data->eigvec, sh_data->eigval, sh_data->N);
}

void SHCompress2(SHData *sh_data, float *source_data, float *compress_data, int d)
{
	float source_temp_data[12] = { 0.0f };

	for (int i = 0; i < 12; i++) {
		source_temp_data[i] = factors12[i] * source_data[i];
	}

	d = d < 1 ? 1 : d;
	d = d < sh_data->N ? d : sh_data->N;

	compress(source_temp_data, sh_data->mean, sh_data->eigvec, sh_data->N, d, compress_data);
}

void SHUncompress2(SHData *sh_data, float *compress_data, float *source_data, int d)
{
	d = d < 1 ? 1 : d;
	d = d < sh_data->N ? d : sh_data->N;

	uncompress(sh_data->mean, sh_data->eigvec, compress_data, sh_data->N, d, source_data);

	for (int i = 0; i < 12; i++) {
		source_data[i] /= factors12[i];
	}
}

void SHCompress3(SHData *sh_data, float *source_data, float *compress_data, int d)
{
	float source_temp_data[27] = { 0.0f };

	for (int i = 0; i < 27; i++) {
		source_temp_data[i] = factors27[i] * source_data[i];
	}

	d = d < 1 ? 1 : d;
	d = d < sh_data->N ? d : sh_data->N;

	compress(source_temp_data, sh_data->mean, sh_data->eigvec, sh_data->N, d, compress_data);
}

void SHUncompress3(SHData *sh_data, float *compress_data, float *source_data, int d)
{
	d = d < 1 ? 1 : d;
	d = d < sh_data->N ? d : sh_data->N;

	uncompress(sh_data->mean, sh_data->eigvec, compress_data, sh_data->N, d, source_data);

	for (int i = 0; i < 27; i++) {
		source_data[i] /= factors27[i];
	}
}
