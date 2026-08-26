#include "Approximation.h"

#include <stdio.h>
#ifdef Q_OS_WIN
#include <process.h>
#endif
#include <math.h>

#include <QString>

//volatile double xx[8] = {100,200,300,400,500,600,700,800};
//volatile double yy[8] = {65.95, 185.95, 301.9, 415, 516, 601, 676.1, 759.1};

double *x, *y, *a, *b, **sums; //
int N = 8;
int K = 3;

void freematrix();
void allocmatrix();
void readmatrix();
void printresult();
void diagonal();

Approximation::Approximation(int polynom_N, std::vector<double> x_v, std::vector<double> y_v) {
    QString title = "Внимание";
    QString text = "Ошибка калибровки";

    if (x_v.size() != y_v.size()) {
        QString detailed_text = "Размерности калибровочных точек не совпадают";
        AQP::warning(nullptr, title, text, detailed_text);
        return;
    }

    if ((size_t)polynom_N >= x_v.size()) {
        QString detailed_text = "Степень полинома должна быть меньше, чем количество калибровочных точек";
        AQP::warning(nullptr, title, text, detailed_text);
        return;
    }

    int i=0,j=0, k=0;

    K = polynom_N;
    N = x_v.size();

    allocmatrix();

    for(k=0; k<N; k++){
        *(x+k) = x_v[k];
        *(y+k) = y_v[k];
    }

    readmatrix();
    //check if there are 0 on main diagonal and exchange rows in that case
    diagonal();
    //fclose(InFile);
    //printmatrix();
    //process rows
    for(k=0; k<K+1; k++){
        for(i=k+1; i<K+1; i++){
            if(sums[k][k]==0){
                printf("\nSolution is not exist.\n");
                return;
            }
            double M = sums[i][k] / sums[k][k];
            for(j=k; j<K+1; j++){
                sums[i][j] -= M * sums[k][j];
            }
            b[i] -= M*b[k];
        }
    }
    //printmatrix();
    for(i=(K+1)-1; i>=0; i--){
        double s = 0;
        for(j = i+1; j<K+1; j++){ //j = i
            s = s + sums[i][j]*a[j];
        }
        a[i] = (b[i] - s) / sums[i][i];
    }
    //printresult();

    _polynom.clear();
    for(i = 0; i <= polynom_N; ++i) {
        _polynom.push_back(a[i]);
    }

    freematrix();
}

Approximation::~Approximation() {

}

//N - number of data points
//K - polinom power
//K<=N
//char filename[256];
//FILE* InFile=NULL;

//void count_num_lines(){
//   //count number of lines in input file - number of equations
//   int nelf=0;       //non empty line flag
//   do{
//       nelf = 0;
//       while(fgetc(InFile)!='\n' && !feof(InFile)) nelf=1;
//       if(nelf) N++;
//   }while(!feof(InFile));
//}

void freematrix(){
    //free memory for matrixes
    int i;
    for(i=0; i<K+1; i++) {
        delete [] sums[i];
    }
    delete [] a;
    delete [] b;
    delete [] x;
    delete [] y;
    delete [] sums;
}

void allocmatrix(){
    //allocate memory for matrixes
    int i,j,k;
    a = new double[K+1];
    b = new double[K+1];
    x = new double[N];
    y = new double[N];

    sums = new double*[K+1];
    if(x==NULL || y==NULL || a==NULL || b==NULL){
        printf("\nNot enough memory to allocate. N=%d, K=%d\n", N, K);
        exit(-1);
    }
    for(i=0; i<K+1; i++){
        sums[i] = new double[K+1];
        if(sums[i]==NULL){
            printf("\nNot enough memory to allocate for %d equations.\n", K+1);
        }
    }
    for(i=0; i<K+1; i++){
        a[i]=0;
        b[i]=0;
        for(j=0; j<K+1; j++){
            sums[i][j] = 0;
        }
    }
    for(k=0; k<N; k++){
        x[k]=0;
        y[k]=0;
    }
}

void readmatrix(){
    int i=0,j=0, k=0;
    //read x, y matrixes from input file
    //   for(k=0; k<N; k++){
    //       fscanf(InFile, "%f", &x[k]);
    //       fscanf(InFile, "%f", &y[k]);
    //   }

    //init square sums matrix
    for(i=0; i<K+1; i++){
        for(j=0; j<K+1; j++){
            sums[i][j] = 0;
            for(k=0; k<N; k++){
                sums[i][j] += pow(x[k], i+j);
            }
        }
    }
    //init free coefficients column
    for(i=0; i<K+1; i++){
        for(k=0; k<N; k++){
            b[i] += pow(x[k], i) * y[k];
        }
    }
}

void printresult(){
   //print polynom parameters
   int i=0;
   printf("\n");
   for(i=0; i<K+1; i++){
       printf("a[%d] = %f\n", i, a[i]);
   }
}
void diagonal(){
    int i, j, k;
    double temp=0;
    for(i=0; i<K+1; i++){
        if(sums[i][i]==0){
            for(j=0; j<K+1; j++){
                if(j==i) continue;
                if(sums[j][i] !=0 && sums[i][j]!=0){
                    for(k=0; k<K+1; k++){
                       temp = sums[j][k];
                       sums[j][k] = sums[i][k];
                       sums[i][k] = temp;
                    }
                    temp = b[j];
                    b[j] = b[i];
                    b[i] = temp;
                    break;
                }
            }
        }
    }
}

void cls(){
   for(int i=0; i<25; i++) printf("\n");
}

void ApproximationTest() {

}

//SECOND SAMPLE CODE
//    {
//    int i,j,k,n,N;
//    n = polinom_N;
//    N = x_v.size();
//    double x[N],y[N];
//    for(i=0; i<N; i++){
//        x[i] = x_v[i];
//        y[i] = y_v[i];
//    }

//    double X[2*n+1];                        //Array that will store the values of sigma(xi),sigma(xi^2),sigma(xi^3)....sigma(xi^2n)
//    for (i=0;i<2*n+1;i++)
//    {
//        X[i]=0;
//        for (j=0;j<N;j++)
//            X[i]=X[i]+pow(x[j],i);        //consecutive positions of the array will store N,sigma(xi),sigma(xi^2),sigma(xi^3)....sigma(xi^2n)
//    }
//    double B[n+1][n+2],a[n+1];            //B is the Normal matrix(augmented) that will store the equations, 'a' is for value of the final coefficients
//    for (i=0;i<=n;i++)
//        for (j=0;j<=n;j++)
//            B[i][j]=X[i+j];            //Build the Normal matrix by storing the corresponding coefficients at the right positions except the last column of the matrix
//    double Y[n+1];                    //Array to store the values of sigma(yi),sigma(xi*yi),sigma(xi^2*yi)...sigma(xi^n*yi)
//    for (i=0;i<n+1;i++)
//    {
//        Y[i]=0;
//        for (j=0;j<N;j++)
//        Y[i]=Y[i]+pow(x[j],i)*y[j];        //consecutive positions will store sigma(yi),sigma(xi*yi),sigma(xi^2*yi)...sigma(xi^n*yi)
//    }
//    for (i=0;i<=n;i++)
//        B[i][n+1]=Y[i];                //load the values of Y as the last column of B(Normal Matrix but augmented)
//    n=n+1;                //n is made n+1 because the Gaussian Elimination part below was for n equations, but here n is the degree of polynomial and for n degree we get n+1 equations

//    for (i=0;i<n;i++)                    //From now Gaussian Elimination starts(can be ignored) to solve the set of linear equations (Pivotisation)
//        for (k=i+1;k<n;k++)
//            if (B[i][i]<B[k][i])
//                for (j=0;j<=n;j++)
//                {
//                    double temp=B[i][j];
//                    B[i][j]=B[k][j];
//                    B[k][j]=temp;
//                }

//    for (i=0;i<n-1;i++)            //loop to perform the gauss elimination
//        for (k=i+1;k<n;k++)
//            {
//                double t=B[k][i]/B[i][i];
//                for (j=0;j<=n;j++)
//                    B[k][j]=B[k][j]-t*B[i][j];    //make the elements below the pivot elements equal to zero or elimnate the variables
//            }
//    for (i=n-1;i>=0;i--)                //back-substitution
//    {                        //x is an array whose values correspond to the values of x,y,z..
//        a[i]=B[i][n];                //make the variable to be calculated equal to the rhs of the last equation
//        for (j=0;j<n;j++)
//            if (j!=i)            //then subtract all the lhs values except the coefficient of the variable whose value                                   is being calculated
//                a[i]=a[i]-B[i][j]*a[j];
//        a[i]=a[i]/B[i][i];            //now finally divide the rhs by the coefficient of the variable to be calculated
//    }

//    _polinom.clear();
//    for(i = 0; i <= polinom_N; ++i) {
//        _polinom.push_back(a[i]);
//    }


