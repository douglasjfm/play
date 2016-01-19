#include "vem.h"

extern gsl_matrix *mtxd2_aux;
extern gsl_matrix *mtxd2;
extern gsl_matrix *tmpscore;
extern gsl_matrix *tmpscore2;

gsl_matrix * expheap[2];
extern double *detL;
extern gsl_matrix **invS;
extern gsl_permutation *permutglobal;

double normaln(gsl_vector *x, gsl_vector *m, gsl_matrix *S, int sk);
double stu(gsl_vector *x, gsl_vector *m, gsl_matrix *S, int sk, double v, int heapid);
double delta(gsl_vector *x, gsl_vector *m, gsl_matrix *S, int heapid);

double score2(gsl_matrix *X, VBGMM *modelo)
{
    int i,k;
    double sum,scor=0;
    expheap[0] = mtxd2;
    expheap[1] = mtxd2_aux;
    for (i=0; i<X->size1; i++)
    {
        gsl_vector_view x = gsl_matrix_row(X,i);
        sum = 0;
        for (k=0; k<modelo->K; k++)
        {
            double pk = vget(modelo->pi,k);

            gsl_vector_view mk = gsl_matrix_column(modelo->xbarra,k);

            sum += pk * normaln(&(x.vector),&(mk.vector),modelo->S[k],k);
        }
        scor += sum;
    }
    return log(scor)/X->size1;
}

double score_aux(gsl_matrix *X, VBGMM *modelo)
{
    int i,k;
    double logAlphaChapeu = log(somatorio(modelo->alpha));
    double sum,scor = 0;

    gsl_matrix *tmp = tmpscore2;
    expheap[0] = mtxd2;
    expheap[1] = mtxd2_aux;

    for (i=0; i<X->size1; i++)
    {
        gsl_vector_view x = gsl_matrix_row(X,i);
        sum = 0;
        for (k=0; k<modelo->K; k++)
        {
            double ak = vget(modelo->alpha,k);
            double bk = vget(modelo->beta,k);
            double vk = vget(modelo->v,k);
            double lk = (vk+1-modelo->dim)*bk/(bk+1);

            gsl_vector_view mk = gsl_matrix_column(modelo->m,k);

            gsl_matrix *Lk = tmp;

            gsl_matrix_memcpy(Lk,modelo->W[k]);
            gsl_matrix_scale(Lk,lk);

            sum += ak * stu(&(x.vector),&(mk.vector),Lk,k,vk+1-modelo->dim,1);
        }
        scor += log(sum) - logAlphaChapeu;
    }

    return scor/X->size1;
}
double score(gsl_matrix *X, VBGMM *modelo)
{
    int i,k;
    double logAlphaChapeu = log(somatorio(modelo->alpha));
    double sum,scor = 0;

    gsl_matrix *tmp = tmpscore;
    expheap[0] = mtxd2;
    expheap[1] = mtxd2_aux;

    for (i=0; i<X->size1; i++)
    {
        gsl_vector_view x = gsl_matrix_row(X,i);
        sum = 0;
        for (k=0; k<modelo->K; k++)
        {
            double ak = vget(modelo->alpha,k);
            double bk = vget(modelo->beta,k);
            double vk = vget(modelo->v,k);
            double lk = (vk+1-modelo->dim)*bk/(bk+1);

            gsl_vector_view mk = gsl_matrix_column(modelo->m,k);

            gsl_matrix *Lk = tmp;

            gsl_matrix_memcpy(Lk,modelo->W[k]);
            gsl_matrix_scale(Lk,lk);

            sum += ak * stu(&(x.vector),&(mk.vector),Lk,k,vk+1-modelo->dim,0);
        }
        scor += log(sum) - logAlphaChapeu;
    }

    return scor/X->size1;
}

double normaln(gsl_vector *x, gsl_vector *m, gsl_matrix *S, int sk)
{
    double a,b,c;
    gsl_vector_view v1 = gsl_matrix_column(expheap[0],0),v2 = gsl_matrix_column(expheap[0],1);

    a = 1/pow((2*PI),(x->size/2));
    b = 1/pow(detL[sk],(0.5));

    gsl_vector_memcpy(&(v1.vector),x);
    gsl_vector_sub(&(v1.vector),m);
    gsl_blas_dgemv(CblasNoTrans,1.0,invS[sk],&(v1.vector),0,&(v2.vector));
    gsl_blas_ddot(&(v1.vector),&(v2.vector),&c);

    c = exp(-c/2);

    return a*b*c;
}

double stu(gsl_vector *x, gsl_vector *m, gsl_matrix *S, int sk, double v, int heapid)
{
    double a,b,c;
    a = gsl_sf_lngamma((x->size+v)/2) - gsl_sf_lngamma(v/2);
    a = exp(a);
    b = sqrt(detL[sk]) * pow((PI*v),(x->size*0.5));
    c = pow((1+delta(x,m,S,heapid)/v),(-(x->size + v)/2));
    return a*b*c;
}

double delta(gsl_vector *x, gsl_vector *m, gsl_matrix *S, int heapid)
{
    double r;
    gsl_matrix* heap = expheap[heapid];
    gsl_vector_view v1 = gsl_matrix_column(heap,0),v2 = gsl_matrix_column(heap,1);
    gsl_vector *dif = &(v1.vector);
    gsl_vector *tmp = &(v2.vector);
    gsl_vector_memcpy(dif,x);
    gsl_vector_sub(dif,m);
    gsl_blas_dgemv(CblasNoTrans,1.0,S,dif,0,tmp);
    gsl_blas_ddot(dif,tmp,&r);
    return r;
}


