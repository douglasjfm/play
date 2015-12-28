
#include "vem.h"

extern gsl_matrix *mtxd2;
extern gsl_matrix *tmpscore;

gsl_matrix * expheap = NULL;

double stu(gsl_vector *x, gsl_vector *m, gsl_matrix *S, double v);
double delta(gsl_vector *x, gsl_vector *m, gsl_matrix *S);

double score(gsl_matrix *X, VBGMM *modelo)
{
    int i,k;
    double logAlphaChapeu = log(somatorio(modelo->alpha));
    double sum,scor = 0;

    gsl_matrix *tmp = tmpscore;
    expheap = mtxd2;

    for (i=0;i<X->size1;i++)
    {
        gsl_vector_view x = gsl_matrix_row(X,i);
        sum = 0;
        for (k=0;k<modelo->K;k++)
        {
            double ak = vget(modelo->alpha,k);
            double bk = vget(modelo->beta,k);
            double vk = vget(modelo->v,k);
            double lk = (vk+1-modelo->dim)*bk/(bk+1);

            gsl_vector_view mk = gsl_matrix_column(modelo->m,k);

            gsl_matrix *Lk = tmp;

            gsl_matrix_memcpy(Lk,modelo->W[k]);
            gsl_matrix_scale(Lk,lk);

            sum += log(ak) + stu(&(x.vector),&(mk.vector),Lk,vk+1-modelo->dim);
        }
        scor += sum - logAlphaChapeu;
        //printf("\n");
    }
    return scor/X->size1;
}

double stu(gsl_vector *x, gsl_vector *m, gsl_matrix *S, double v)
{
    double a,b,c;
    a = gsl_sf_lngamma((x->size+v)/2) - gsl_sf_lngamma(v/2);
    b = log(sqrt(determinante(S))) + log(pow((M_PI*v),(x->size*0.5)));
    c = log(1+delta(x,m,S)/v) * (-(x->size + v)/2);
    //printf("%03.0lf ",a+b+c);
    return a+b+c;
}

double delta(gsl_vector *x, gsl_vector *m, gsl_matrix *S)
{
    double r;
    gsl_vector_view v1 = gsl_matrix_column(expheap,0),v2 = gsl_matrix_column(expheap,1);
    gsl_vector *dif = &(v1.vector);
    gsl_vector *tmp = &(v2.vector);
    gsl_vector_memcpy(dif,x);
    gsl_vector_sub(dif,m);
    gsl_blas_dgemv(CblasNoTrans,1.0,S,dif,0,tmp);
    gsl_blas_ddot(dif,tmp,&r);
    return r;
}


