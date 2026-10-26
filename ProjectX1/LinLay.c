/*Run with clang -O1
because clang -O0 is too messy and
clang -O2 is doing the optimizations itself!!
*/
#include<stdio.h>

typedef struct{
  int rows;
  int cols;
  int nnz;
  float *val;
  int *c_idx;
  int *r_ptr;
}CSRM;

void linLay(const float *X, const float *W, const float *b, float *Y, int M, int N, int K){
  for(int i=0;i<M;i++){
    for(int j=0;j<N;j++){
      float sum = b[j];
      for(int k=0;k<K;k++){
        sum += X[i*K+k]*W[k*N+j];
      }
      Y[i*N+j] = sum;
    }
  }
}

int main(){

  float X[] = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f};
  float W[] = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f};
  float b[] = {1.0f, 2.0f};

  float Y[4] = {0};
  int M=2, K=3, N=2;

  linLay(X,W,b,Y,M,N,K);
  printf("Y=\n");
  for(int i=0;i<M;i++){
    for(int j=0;j<N;j++){
      printf("%.1f ", Y[i*N+j]);
    }
    printf("\n");
  }
  return 0;
}