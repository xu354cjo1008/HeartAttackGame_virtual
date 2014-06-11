//
//  matrix.h
//  
//
//  Created by 李勁璋 on 2014/2/9.
//
//

#ifndef _matrix_h
#define _matrix_h

#define M(x,i,j) *(x->table + i * x->cols + j)

typedef struct {
    int rows;
    int cols;
    int total;
    double *table;
} Matrix;

//建立矩陣：
Matrix *matrix_create(int rows, int cols);
//矩陣最佳化：
Matrix *matrix_optimize(Matrix *matrix);
//解矩陣的方程式：
Matrix *matrix_solution(Matrix *matrix);
//顯示並列出矩陣內容：
void matrix_print(Matrix *matrix);
//清空矩陣：
void matrix_clear(Matrix *matrix);

Matrix *matrix_identity(int x);

Matrix *matrix_transport(Matrix *x);

Matrix *matrix_inverse_second_orders(Matrix *x);

Matrix *matrix_add(Matrix *x, Matrix *y);

Matrix *matrix_min(Matrix *x, Matrix *y);

Matrix *matrix_multiple(Matrix *x, Matrix *y);

#endif
