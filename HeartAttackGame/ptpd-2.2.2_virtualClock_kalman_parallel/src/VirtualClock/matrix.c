//
//  matrix.c
//  
//
//  Created by 李勁璋 on 2014/2/9.
//
//

#include <stdio.h>
#include <stdlib.h>
#include "matrix.h"

void matrix_clear(Matrix *matrix)
{
    int i;
    double *ptr;
    
    /* initializing matrix */
    ptr = matrix->table;
    for (i=0;i<matrix->total;i++)
        *(ptr++) = 0;
}

Matrix *matrix_create(int rows, int cols)
{
    Matrix *matrix;
    
    /* allocate */
    matrix = (Matrix *)malloc(sizeof(Matrix));
    if (!matrix) {
        printf("matrix alloc fail!\n");
        return NULL;
    }
    matrix->rows = rows;
    matrix->cols = cols;
    matrix->total = rows * cols;
    matrix->table = (double *)malloc(sizeof(double)*rows*cols);
    matrix_clear(matrix);
    
    return matrix;
}

Matrix *matrix_optimize(Matrix *matrix)
{
    Matrix *new_matrix;
    int i;
    double *ptr, *o_ptr;
    
    /* create a new matrix */
    new_matrix = matrix_create(matrix->rows, matrix->cols);
    
    /* copy old matrix to the new one */
    ptr = new_matrix->table;
    o_ptr = matrix->table;
    
    for (i=0;i<matrix->total;i++)
        *(ptr++) = *(o_ptr++);
    
    return new_matrix;
}

Matrix *matrix_solution(Matrix *matrix)
{
    Matrix *result;
    Matrix *source;
    int i, j, k;
    double rate;
    double *ptr;
    
    /* 對矩陣做最佳化 */
    source = matrix_optimize(matrix);
    
    ptr = source->table;
    for (i=0;i<source->rows-1;i++) {
        if (*(ptr+i*source->cols+i)==0) {
            /* ignore the column */
            continue;
        }
        
        for (j=i+1;j<source->rows;j++) {
            rate = *(ptr+j*source->cols+i) / *(ptr+i*source->cols+i);
            *(ptr+j*(source->cols)+i) = 0;
            
            for (k=i+1;k<source->cols;k++) {
                *(ptr+j*source->cols+k) -= *(ptr+i*source->cols+k) * rate;
            }
        }
    }
    
    for (i=source->rows-1;i>0;i--) {
        if (*(ptr+i*source->cols+i)==0)
            continue;
        
        for (j=i-1;j>=0;j--) {
            rate = *(ptr+j*(source->cols)+i) / *(ptr+i*source->cols+i);
            *(ptr+j*(source->cols)+i) = 0;
            *(ptr+j*(source->cols)+source->cols-1) -= *(ptr+i*source->cols+source->cols-1) * rate;
        }
    }
    
    /* 建立空白的矩陣 */
    result = matrix_create(source->rows, 1);
    
    ptr = source->table;
    for (i=0;i<source->rows;i++) {
        if (*(source->table+i*source->cols+i)==0)
            continue;
        
        *(result->table+i) = *(ptr+i*source->cols+source->cols-1) / *(source->table+i*source->cols+i);
    }
    
    /* release */
    free(source->table);
    free(source);
    
    return result;
}

Matrix *matrix_identity(int x)
{
    Matrix *c = matrix_create(x, x);
    for (int i = 0; i < c->rows; i++) {
        M(c,i,i) = 1;
    }
    return c;
}

Matrix *matrix_transport(Matrix *x)
{
   Matrix *c = matrix_create(x->cols, x->rows);
   for (int i = 0; i < c->rows; i++) {
       for (int j = 0; j < c->cols; j++) {
        //   *(c->table + i * c->cols + j) = *(x->table + j * x->rows + i);
           M(c,i,j) = M(x,j,i);
       }
   }
    return c;
}

Matrix *matrix_inverse_second_orders(Matrix *x)
{
    if (x->rows != x->cols) {
        printf("Matrix dimension mismatch.\n");
        return NULL;
    }
    Matrix *c = matrix_create(x->cols, x->rows);
    double temp = M(x,0,0) * M(x,1,1) - M(x,0,1) * M(x,1,0);
    M(c,0,0) = M(x,1,1)/temp;
    M(c,0,1) = -M(x,1,0)/temp;
    M(c,1,0) = -M(x,0,1)/temp;
    M(c,1,1) = M(x,0,0)/temp;
    return c;
}

Matrix *matrix_add(Matrix *x, Matrix *y)
{
    // 檢查兩矩陣的大小是否能相加
    if ((x->rows != y->rows) || (x->cols != y->cols)) {
        printf("Matrix add dimension mismatch.\n");
        return NULL;
    }
    Matrix *c = matrix_create(x->rows, y->cols);
    for (int i = 0; i < x->rows; i++) {
        for (int j = 0; j < x->cols; j++) {
         //   *(c->table + i * c->cols + j) = *(x->table + i * x->cols + j) + *(y->table + i * y->cols + j)
            M(c,i,j) = M(x,i,j) + M(y,i,j);
        }
    }
    return c;
}

Matrix *matrix_min(Matrix *x, Matrix *y)
{
    // 檢查兩矩陣的大小是否能相加
    if ((x->rows != y->rows) || (x->cols != y->cols)) {
        printf("Matrix minius dimension mismatch.\n");
        return NULL;
    }
    Matrix *c = matrix_create(x->rows, y->cols);
    for (int i = 0; i < x->rows; i++) {
        for (int j = 0; j < x->cols; j++) {
            //   *(c->table + i * c->cols + j) = *(x->table + i * x->cols + j) + *(y->table + i * y->cols + j)
            M(c,i,j) = M(x,i,j) - M(y,i,j);
        }
    }
    return c;
}

Matrix *matrix_multiple(Matrix *x, Matrix *y)
{
    if (x->cols != y->rows) {
        printf("Matrix multiple dimension mismatch.\n");
        return NULL;
    }
    Matrix *c = matrix_create(x->rows, y->cols);
    //double *ptr = c->table;
    double temp;
    for (int i = 0; i < x->rows; i++) {
        for (int j = 0; j < y->cols; j++) {
            temp = 0;
            for (int k = 0; k < x->cols; k++) {
                //temp += *(x->table + i * x->cols + k) * *(y->table + k * y->cols + j);
                temp += M(x,i,k) * M(y,k,j);
            }
           // *(ptr + i * x->cols + j) = temp;
            M(c,i,j) = temp;
        }
    }
    return c;
}
