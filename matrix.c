#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <float.h>
#include <math.h>
#include <string.h>
#include <assert.h>
#include "matrix.h"

#define MAX(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })

#define MIN(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })

double **
init_data(unsigned int m, unsigned int n) {
    double **data = (double **)malloc(sizeof(double *) * m);
    for (int i = 0; i < m; i++) {
        data[i] = (double *)malloc(sizeof(double) * n);
    }

    return data;
}

matrix *
matrix_create_empty(unsigned int m, unsigned int n) {
    matrix *M = (matrix *)malloc(sizeof(matrix));
    matrix s = (matrix){NULL, m, n};
    memcpy(M, &s, sizeof(matrix));

    M->data = init_data(m, n);

    return M;
}

matrix *
matrix_create_va(unsigned int m, unsigned int n, ...) {
    va_list ap;
    matrix *M = NULL;
    if ((M = matrix_create_empty(m, n))) {
        va_start(ap, n);
        for (unsigned int i = 0; i < m; i++) {
            for (unsigned int j = 0; j < n; j++) {
                M->data[i][j] = va_arg(ap, double);
            }
        }

        va_end(ap);
    }
    return M;
}

matrix *
matrix_create_zeros(unsigned int m, unsigned int n) {
    matrix *M = NULL;
    if ((M = matrix_create_empty(m, n))) {
        matrix_fill(M, 0);
    }

    return M;
}

matrix *
matrix_create_ones(unsigned int m, unsigned int n) {
    matrix *M = NULL;
    if ((M = matrix_create_empty(m, n))) {
        matrix_fill(M, 1);
    }

    return M;
}

matrix *
matrix_copy(const matrix *A) {
    matrix *B = matrix_create_empty(A->m, A->n);
    for (int i = 0; i < B->m; i++) {
        for (int j = 0; j < B->n; j++) {
            B->data[i][j] = A->data[i][j];
        }
    }

    return B;
}

matrix *
matrix_mult_scalar(const matrix *A, double scalar) {
    matrix *B = matrix_copy(A);
    matrix_mult_scalar_inplace(B, scalar);

    return B;
}

matrix *
matrix_add(const matrix *A, const matrix *B) {
    if (!matrix_same_order(A, B)) {
        return NULL;
    }

    matrix *C = matrix_copy(A);
    matrix_add_inplace(C, B);

    return C;
}

matrix *
matrix_sub(const matrix *A, const matrix *B) {
    if (!matrix_same_order(A, B)) {
        return NULL;
    }

    matrix *C = matrix_copy(A);
    matrix_sub_inplace(C, B);

    return C;
}

matrix *
matrix_mult(const matrix *A, const matrix *B) {
    if (!matrix_can_mult(A, B)) {
        return NULL;
    }

    matrix *C = matrix_create_zeros(A->m, B->n);
    for (int i = 0; i < C->m; i++) {
        for (int j = 0; j < C->n; j++) {
            for (int k = 0; k < A->n; k++) {
                C->data[i][j] += A->data[i][k] * B->data[k][j];
            }
        }
    }
    return C;
}

void
matrix_mult_scalar_inplace(matrix *A, double scalar) {
    for (int i = 0; i < A->m; i++) {
        for (int j = 0; j < A->n; j++) {
            A->data[i][j] *= scalar;
        }
    }
}

void
matrix_add_inplace(matrix *A, const matrix *B) {
    assert(matrix_same_order(A, B));
    if (!matrix_same_order(A, B)) {
        return;
    }

    for (int i = 0; i < A->m; i++) {
        for (int j = 0; j < A->n; j++) {
            A->data[i][j] += B->data[i][j];
        }
    }
}

void
matrix_sub_inplace(matrix *A, const matrix *B) {
    assert(matrix_same_order(A, B));
    if (!matrix_same_order(A, B)) {
        return;
    }

    for (int i = 0; i < A->m; i++) {
        for (int j = 0; j < A->n; j++) {
            A->data[i][j] -= B->data[i][j];
        }
    }
}

int
matrix_same_order(const matrix *A, const matrix *B) {
    return A->m == B->m && A->n == B->n;
}

int
matrix_can_mult(const matrix *A, const matrix *B) {
    return A->n == B->m;
}

int
matrix_is_square(const matrix *A) {
    return A->m == A->n;
}

int
matrix_eq(const matrix *A, const matrix *B) {
    if (A == B) {
        return 1;
    }
    if (!matrix_same_order(A, B)) {
        return 0;
    }

    for (int i = 0; i < A->m; i++) {
        for (int j = 0; j < A->n; j++) {
            if (fabs(A->data[i][j] - B->data[i][j]) > DBL_EPSILON) {
                return 0;
            }
        }
    }

    return 1;
}

int
matrix_is_singular(const matrix *A) {
    return fabs(matrix_determinant(A)) <= DBL_EPSILON;
}

matrix *
matrix_transpose(const matrix *A) {
    matrix *T = matrix_create_empty(A->n, A->m);
    for (int i = 0; i < T->m; i++) {
        for (int j = 0; j < T->n; j++) {
            T->data[i][j] = A->data[j][i];
        }
    }

    return T;
}

matrix *
matrix_diagonal(const matrix *A) {
    int dim = MIN(A->m, A->n);

    matrix *diagonal = matrix_create_empty(1, dim);
    for (int i = 0; i < dim; i++) {
        diagonal->data[0][i] = A->data[i][i];
    }

    return diagonal;
}

matrix *
matrix_row(const matrix *A, unsigned int r) {
    matrix *row = matrix_create_empty(1, A->n);

    for (int i = 0; i < A->n; i++) {
        row->data[0][i] = A->data[r][i];
    }

    return row;
}

matrix *
matrix_col(const matrix *A, unsigned int c) {
    matrix *col = matrix_create_empty(A->m, 1);

    for (int i = 0; i < A->m; i++) {
        col->data[i][0] = A->data[i][c];
    }

    return col;
}

double
matrix_minor(const matrix *A, unsigned int i, unsigned int j) {
    if (!matrix_is_square(A)
        || A->n <= 1
        || i >= A->m
        || j >= A->n) {
        return 0;
    }
    matrix *B = matrix_delete_row(A, i);
    matrix *C = matrix_delete_col(B, j);

    double retVal = matrix_determinant(C);
    matrix_free(B);
    matrix_free(C);
    return retVal;
}

double
matrix_cofactor(const matrix *A, unsigned int i, unsigned int j) {
    double minor = matrix_minor(A, i, j);
    int isEven = ((i + j) % 2 == 0);
    double multiplier = isEven ? 1.0 : -1.0;
    return  multiplier * minor;
}

double
matrix_determinant_1(const matrix *A) {
    return A->data[0][0];
}

double
matrix_determinant_2(const matrix *A) {
    double a = A->data[0][0];
    double b = A->data[0][1];
    double c = A->data[1][0];
    double d = A->data[1][1];
    return a * d - b * c;
}

double
matrix_determinant_3(const matrix *A) {
    double a = A->data[0][0];
    double b = A->data[0][1];
    double c = A->data[0][2];
    double d = A->data[1][0];
    double e = A->data[1][1];
    double f = A->data[1][2];
    double g = A->data[2][0];
    double h = A->data[2][1];
    double i = A->data[2][2];
    return (
          a * e * i
        + b * f * g
        + c * d * h
        - c * e * g
        - b * d * i
        - a * f * h
        );
}

double
matrix_determinant_smaller_cases(const matrix *A) {
    double (*det)(const matrix *) = NULL;
    switch (A->n) {
        case 1: det = matrix_determinant_1; break;
        case 2: det = matrix_determinant_2; break;
        case 3: det = matrix_determinant_3; break;
    }
    return det(A);
}

double matrix_determinant_n(const matrix *A) {
    if (A->n <= 3) {
        return matrix_determinant_smaller_cases(A);
    }

    double acc = 0;
    int j = 0;
    for (int i = 0; i < A->n; i++) {
        double v = A->data[i][j];
        if (v == 0) {
            continue;
        }

        acc += v * matrix_cofactor(A, i, j);
    }
    return acc;
}

double
matrix_determinant(const matrix *A) {
    if (!matrix_is_square(A)) {
        return 0;
    }

    return matrix_determinant_n(A);
}

double
matrix_norm_1(const matrix *A) {
    double max_sum = 0;
    for (int j = 0; j < A->n; j++) {
        double sum = 0;
        for (int i = 0; i < A->m; i++) {
            sum += fabs(A->data[i][j]);
        }
        if (sum > max_sum) {
            max_sum = sum;
        }
    }
    return max_sum;
}

double
matrix_norm_inf(const matrix *A) {
    double max_sum = 0;
    for (int i = 0; i < A->m; i++) {
        double sum = 0;
        for (int j = 0; j < A->n; j++) {
            sum += fabs(A->data[i][j]);
        }
        if (sum > max_sum) {
            max_sum = sum;
        }
    }
    return max_sum;
}

matrix *
matrix_delete_col(const matrix *A, unsigned int col) {
    if (col >= A->n) {
        return NULL;
    }

    matrix *B = matrix_create_empty(A->m, A->n - 1);

    for (int ai = 0, bi = 0; ai < A->m; ai++) {
        for (int aj = 0, bj = 0; aj < A->n; aj++) {
            if (aj == col) {
                continue;
            }
            B->data[bi][bj] = A->data[ai][aj];
            bj++;
        }
        bi++;
    }

    return B;
}

matrix *
matrix_delete_row(const matrix *A, unsigned int row) {
    if (row >= A->m) {
        return NULL;
    }

    matrix *B = matrix_create_empty(A->m - 1, A->n);

    for (int ai = 0, bi = 0; ai < A->m; ai++) {
        if (ai == row) {
                continue;
            }
        for (int aj = 0, bj = 0; aj < A->n; aj++) {
            B->data[bi][bj] = A->data[ai][aj];
            bj++;
        }
        bi++;
    }

    return B;
}

void
matrix_swap_col_inplace(matrix *A, unsigned int c1, unsigned int c2) {
    if (c1 >= A->n || c2 >= A->n || c1 == c2) {
        return;
    }

    for (int i = 0; i < A->n; i++) {
        double temp = A->data[i][c1];
        A->data[i][c1] = A->data[i][c2];
        A->data[i][c2] = temp;
    }
}

void
matrix_swap_row_inplace(matrix *A, unsigned int r1, unsigned int r2) {
    if (r1 >= A->m || r2 >= A->m || r1 == r2) {
        return;
    }

    for (int i = 0; i < A->n; i++) {
        double temp = A->data[r1][i];
        A->data[r1][i] = A->data[r2][i];
        A->data[r2][i] = temp;
    }
}

void
matrix_map(matrix *A, double (*f)(double)) {
    for (int i = 0; i < A->m; i++) {
        for (int j = 0; j < A->n; j++) {
            A->data[i][j] = f(A->data[i][j]);
        }
    }
}

void
matrix_fill(matrix *A, double v) {
    for (int i = 0; i < A->m; i++) {
        for (int j = 0; j < A->n; j++) {
            A->data[i][j] = v;
        }
    }
}

void
matrix_print(const matrix *M) {
    for (int i = 0; i < M->m; i++) {
        for (int j = 0; j < M->n; j++) {
            printf("%lf\t", M->data[i][j]);
        }
        printf("\n");
    }
}

void
matrix_free(matrix *M) {
    for (int i = 0; i < M->m; i++) {
        free(M->data[i]);
    }
    free(M->data);
    free(M);
}
