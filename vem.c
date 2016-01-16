/* Variational Expectation Maximization for Gaussian Mixture Models.
Copyright (C) 2012-2015 Douglas Medeiros

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details. */

#include "vem.h"

extern double *detL;
extern gsl_matrix *tmpscore2;
extern gsl_matrix *tmpscore3;
extern gsl_matrix **invS;
extern gsl_matrix *mtxd2;
extern gsl_permutation *permutglobal;

VBGMM* VBGMM_alloc(number k, number d)
{
    VBGMM *mdl;
    mdl = (VBGMM*) calloc(1,sizeof(VBGMM));
    mdl->alpha = NULL;
    mdl->beta = NULL;
    mdl->v = NULL;
    mdl->m = NULL;
    mdl->W = NULL;
    mdl->L = gsl_vector_alloc(VBGMMMAXITER);
    gsl_vector_set_zero(mdl->L);
    mdl->K = k;
    mdl->dim = d;
    detL = (double*) calloc(k,sizeof(double));
    return mdl;
}

void vbg_delete(VBGMM *m)
{
    int i;
    for (i=0;i<m->K;i++)
    {
        gsl_matrix_free(m->W[i]);
        gsl_matrix_free(m->S[i]);
        gsl_matrix_free(invS[i]);
    }
    gsl_matrix_free(m->xbarra);
    gsl_matrix_free(m->m);
    gsl_vector_free(m->alpha);
    gsl_vector_free(m->beta);
    gsl_vector_free(m->v);
    gsl_vector_free(m->L);
    gsl_vector_free(m->pi);
    free(m->W);
    free(m->S);
    free(m);
    free(detL);
    free(invS);
}

void saveVGMM(char *fname,VBGMM *m)
{
    FILE *f = fopen(fname,"w");
    int i;
    fprintf(f,"%d %d\n",m->K,m->dim);
    gsl_vector_fprintf(f,m->alpha,"%.10f");
    fprintf(f,"\n");
    gsl_vector_fprintf(f,m->beta,"%.10f");
    fprintf(f,"\n");
    gsl_vector_fprintf(f,m->v,"%.10f");
    fprintf(f,"\n");
    gsl_matrix_fprintf(f,m->m,"%.10f");
    fprintf(f,"\n");
    for(i=0;i<m->K;i++)
        gsl_matrix_fprintf(f,m->W[i],"%.10f");

    fprintf(f,"\n\n");
    i=0;
    while(i < m->L->size && ((vget(m->L,i) < -0.0001) || (vget(m->L,i) > 0.0001)))
    {
        fprintf(f,"%.5f\n",vget(m->L,i));
        i++;
    }
    fclose(f);
}

/*! Adianta o calculo do Det de L || S, para a dist. de Stuart || Gaussiana*/
void calcDetL(VBGMM *modelo, char modo)
{
    int k;
    gsl_matrix *tmp = tmpscore2;
    /*Adianta o calculo det Lk*/
    for(k=0; k<modelo->K; k++)
    {
        double bk = vget(modelo->beta,k);
        double vk = vget(modelo->v,k);
        double lk = (vk+1-modelo->dim)*bk/(bk+1);

        gsl_matrix *Lk = tmp;

        if (modo == 's')
        {
            gsl_matrix_memcpy(Lk,modelo->W[k]);
            gsl_matrix_scale(Lk,lk);
        }
        else
            gsl_matrix_memcpy(Lk,modelo->S[k]);

        detL[k] = determinante(Lk);
    }
}

/*! Adianta o calculo da Inv de S, para agilizar a dist. Normal*/
void calcInvS(VBGMM *modelo)
{
    int k,s;
    gsl_permutation *perm = permutglobal;
    gsl_matrix *is, *tmp = tmpscore2, *id = tmpscore3;
    invS = (gsl_matrix**) calloc(modelo->K,sizeof(gsl_matrix*));
    gsl_matrix_set_identity(id);
    for(k=0; k<modelo->K; k++)
    {
        is = gsl_matrix_alloc(modelo->dim,modelo->dim);
        gsl_matrix_memcpy(tmp,modelo->S[k]);
        gsl_matrix_mul_elements(tmp,id);
        gsl_linalg_LU_decomp (tmp, perm, &s);
        gsl_linalg_LU_invert (tmp, perm, is);
        invS[k] = is;
    }
}

double mtrace (gsl_matrix *m)
{
    int i;
    double r=1;
    if (m->size1 != m->size2)
    {
        printf("mtrace: matriz nao eh quadrada!\n");
        exit(0xE);
    }
    for (i=0; i<m->size1; i++)
        r *= mget(m,i,i);
    return r;
}

double determinante (gsl_matrix *mtx)
{
    int s;
    gsl_matrix *m;
    gsl_permutation *p;
    if (mtx->size1 != mtx->size2)
    {
        printf("determinate: nao eh quadrada!\n");
        exit(0xE);
    }
    p = permutglobal;
    m = tmpscore2;
    gsl_matrix_memcpy(m,mtx);
    gsl_linalg_LU_decomp(m,p,&s);
    return gsl_linalg_LU_det(m,s);
}

gsl_matrix* inver (gsl_matrix *m)
{
    gsl_matrix *r = gsl_matrix_alloc(m->size1,m->size2);
    int i;
    gsl_matrix_set_identity(r);
    for (i=0; i<m->size1; i++)
        gsl_matrix_set(r,i,i,1/gsl_matrix_get(m,i,i));
    return r;
}

gsl_matrix* matrix_from_vec_x_vec (gsl_vector *a, gsl_vector *b, double aa, double bb, gsl_matrix* r)
{
    gsl_matrix *ma,*mb;
    gsl_vector_view va,vb;
    gsl_vector_scale(a,aa);
    gsl_vector_scale(b,bb);
    ma = gsl_matrix_alloc(a->size,1);
    mb = gsl_matrix_alloc(1,b->size);
    va = gsl_matrix_column(ma,0);
    vb = gsl_matrix_row(mb,0);
    gsl_vector_memcpy(&(va.vector),a);
    gsl_vector_memcpy(&(vb.vector),b);
    gsl_blas_dgemm(CblasNoTrans,CblasNoTrans,1,ma,mb,0,r);
    gsl_matrix_free(ma);
    gsl_matrix_free(mb);
    return r;
}

double somatorio (gsl_vector *v)
{
    double s=0;
    gsl_vector *ones;
    ones = gsl_vector_alloc(v->size);
    gsl_vector_set_all(ones,1.0);
    gsl_blas_ddot(v,ones,&s);
    gsl_vector_free(ones);
    return s;
}

typedef struct EARG_T
{
    int K_;
    int D_;
    int s1_;
    int s2_;
    gsl_vector *v_;
    gsl_vector *beta_;
    gsl_vector *logLambdaTilde_;
    gsl_matrix **W_;
    gsl_matrix *m_;
    gsl_matrix *E_;
    gsl_matrix *dado_;
    int *f;
}EARG_T;

typedef struct rARG_T
{
    int K_;
    int N_;
    int s1_;
    int s2_;
    gsl_vector *rhoexprow_;
    gsl_matrix *logRho_;
    gsl_matrix *logr_;
    gsl_matrix *r_;
    int *f;
}rARG_T;

void* rcomp (rARG_T *arg)
{
    int i,j,K = arg->K_,s1 = arg->s1_,s2 = arg->s2_;
    gsl_vector *rhoexprow = arg->rhoexprow_;
    gsl_matrix *logRho = arg->logRho_, *logr = arg->logr_, *r = arg->r_;

    for(i=s1; i<s2; i++)
        for (j=0; j<K; j++)
            vset(rhoexprow,i,vget(rhoexprow,i) + exp(mget(logRho,i,j)));

    for (i=s1; i<s2; i++)
        for (j=0; j<logRho->size2; j++)
        {
            mset(logr,i,j,mget(logRho,i,j)-log(vget(rhoexprow,i)));
            mset(r,i,j,exp(mget(logr,i,j)));
        }
    *(arg->f) = 1;
    return NULL;
}

void* Ecomp (EARG_T *arg)
{
    int c;

    int K = arg->K_, D = arg->D_, s1 = arg->s1_, s2 = arg->s2_;
    gsl_vector* v, *beta;
    gsl_matrix **W, *m, *E;
    gsl_matrix *x;

    v = arg->v_;
    beta = arg->beta_;
    W = arg->W_;
    m = arg->m_;
    E = arg->E_;
    x = arg->dado_;

    gsl_vector *difxm, *difxm2;

    for (c=0; c<K; c++)
    {
        int n;
        gsl_vector_view mkcol,vx;

        difxm = gsl_vector_alloc(D);
        difxm2 = gsl_vector_alloc(D);
        mkcol = gsl_matrix_column(m,c);
        for (n=s1; n<s2; n++)
        {
            double enk;
            vx = gsl_matrix_column(x,n);
            gsl_vector_memcpy(difxm,&(vx.vector));

            gsl_vector_sub(difxm,&(mkcol.vector));

            gsl_blas_dgemv(CblasNoTrans,1.0,W[c],difxm,0.0,difxm2);

            gsl_blas_ddot(difxm,difxm2,&enk);

            enk *= vget(v,c);
            enk += D/vget(beta,c);

            gsl_matrix_set(E,n,c,enk);
        }
    }
    gsl_vector_free(difxm);
    gsl_vector_free(difxm2);
    *(arg->f) = 1;
    return NULL;
}

void vem_train (VBGMM *vbg, gmm *gm, data *dado, double alpha0, double beta0, double v0, gsl_vector *m0, gsl_matrix *W0)
{
    int i,j,k,t;
    int D = dado->dimension, N = 1500;//dado->samples;
    int K = gm->num;
    double likIncr = THRES+10.0,constant = D*log(2);
    gsl_vector *logLambdaTilde = gsl_vector_alloc(K);
    gsl_matrix *E = gsl_matrix_alloc(N,K);
    gsl_vector *trSW = gsl_vector_alloc(K);
    gsl_vector *xbarWxbar = gsl_vector_alloc(K);
    gsl_vector *mWm = gsl_vector_alloc(K);
    gsl_vector *trW0invW = gsl_vector_alloc(K);

    gsl_vector *alpha, *beta, *v, *pi;
    gsl_matrix **W, *invW0, *m, *m2;

    gsl_matrix *x = gsl_matrix_alloc(D,N), *IDxD;

    gsl_vector *onesD, *onesK, *onesN;

    for (i=0; i<D; i++)
        for (j=0; j<N; j++)
            mset(x,i,j,dado->data[j][i]);

    gsl_vector *Nk = gsl_vector_alloc(K);
    gsl_matrix *xbar = gsl_matrix_alloc(D,K);
    gsl_matrix **S;
    S = (gsl_matrix**) calloc(K,sizeof(gsl_matrix*));

    for (i=0; i<K; i++)
        S[i] = gsl_matrix_alloc(D,D);

    for (i=0; i<K; i++)
        gsl_vector_set(Nk,i,N*exp(gm->mix[i].prior));

    for (i=0; i<K; i++)
    {
        gsl_matrix_set_identity(S[i]);

        for(j=0; j<D; j++)
        {
            gsl_matrix_set(S[i],j,j,gm->mix[i].dcov[j]);
            gsl_matrix_set(xbar,j,i,gm->mix[i].mean[j]);
        }
    }

    alpha = gsl_vector_alloc(K);
    gsl_vector_set_all(alpha,alpha0);
    gsl_vector_add(alpha,Nk);

    beta = gsl_vector_alloc(K);
    gsl_vector_set_all(beta,beta0);
    gsl_vector_add(beta,Nk);

    v = gsl_vector_alloc(K);
    gsl_vector_set_all(v,v0);
    gsl_vector_add(v,Nk);

    pi = gsl_vector_alloc(K);

    W = (gsl_matrix**) calloc(K,sizeof(gsl_matrix*));
    invW0 = inver(W0);

    onesD = gsl_vector_alloc(D);
    onesK = gsl_vector_alloc(K);
    onesN = gsl_vector_alloc(N);
    gsl_vector_set_all(onesD,1);
    gsl_vector_set_all(onesK,1);
    gsl_vector_set_all(onesN,1);

    m = gsl_matrix_alloc(D,K);
    m2 = gsl_matrix_alloc(D,K);

    m = matrix_from_vec_x_vec(m0,onesK,beta0,1.0,m);
    m2 = matrix_from_vec_x_vec(onesD,Nk,1.0,1.0,m2);

    gsl_matrix_mul_elements(m2,xbar);

    gsl_matrix_add(m,m2);

    m2 = matrix_from_vec_x_vec(onesD,beta,1.0,1.0,m2);

    gsl_matrix_div_elements(m,m2);

    IDxD = gsl_matrix_alloc(D,D);
    gsl_matrix_set_identity(IDxD);
    for (k=0; k<K; k++)
    {
        int s;
        double mult1 = beta0*gsl_vector_get(Nk,k)/(beta0+gsl_vector_get(Nk,k));

        gsl_vector_view diff3_ = gsl_matrix_column(xbar,k);
        gsl_vector *diff3 = gsl_vector_alloc(diff3_.vector.size);
        gsl_matrix *Sk = gsl_matrix_alloc(S[k]->size1,S[k]->size2), *Qdiff3;

        gsl_vector_memcpy(diff3,&(diff3_.vector));
        gsl_vector_sub(diff3,m0);
        gsl_matrix_memcpy(Sk,S[k]);
        gsl_matrix_scale(Sk,vget(Nk,k));
        Qdiff3 = gsl_matrix_alloc(diff3->size,diff3->size);
        Qdiff3 = matrix_from_vec_x_vec(diff3,diff3,1,1,Qdiff3);
        gsl_matrix_scale(Qdiff3,mult1);

        gsl_matrix_add(Sk,Qdiff3);
        gsl_matrix_add(Sk,invW0);

        W[k] = gsl_matrix_alloc(D,D);
        gsl_permutation *perm = permutglobal;
        gsl_linalg_LU_decomp (Sk, perm, &s);
        gsl_linalg_LU_invert (Sk, perm, W[k]);//gsl_matrix_fprintf(stdout,W[k],"%.15f");
        //gsl_matrix_mul_elements(W[k],IDxD);

        gsl_matrix_free(Sk);
        gsl_matrix_free(Qdiff3);
        gsl_vector_free(diff3);
    }

    for (t=0; t<VBGMMMAXITER; t++)
    {
        double psiAlphaHat, sumAlpha;
        double logCalpha0, logB0, logCalpha, DlogBeta0;
        double Lt1,Lt2,Lt3,Lt4,Lt41,Lt42,Lt5,Lt6,Lt7;
        int c;

        gsl_vector *logPiTil, *t1, *alphgamln;
        gsl_vector *vdiffD1, *vdiffD2, *HqLambda;

        gsl_permutation *permD;

        gsl_matrix *logRho = gsl_matrix_alloc(E->size1,E->size2);
        gsl_matrix *r = gsl_matrix_alloc(E->size1,E->size2);
        gsl_matrix *logr = gsl_matrix_alloc(E->size1,E->size2);
        gsl_vector *rhoexprow = gsl_vector_alloc(K);
        gsl_matrix *mKxN = gsl_matrix_alloc(K,N);
        gsl_matrix *diff1, *diff2;
        gsl_matrix *rkq = gsl_matrix_alloc(D,N);
        gsl_matrix *mNxD = gsl_matrix_alloc(N,D);
        gsl_matrix *mDxD = gsl_matrix_alloc(D,D);

        gsl_vector_view visu;

        int f1 = 0,f2 = 0;
        EARG_T arg1, arg2;
        pthread_t tid1,tid2;
        pthread_attr_t tattr1, tattr2;

        sumAlpha = somatorio(alpha);
        psiAlphaHat = gsl_sf_psi(sumAlpha);

        logPiTil = gsl_vector_alloc(alpha->size);

        t1 = gsl_vector_alloc(D);

        ///Eqs. 10.65, 10.66
        for (c=0; c<alpha->size; c++)
        {
            for (i=0;i<D;i++)
            {
                double vk = (vget(v,c)-i)/2;
                double t1v = gsl_sf_psi(vk);
                if (GSL_NAN == t1v)
                {
                    printf("vem.c: NAN - %d %d",c,i);
                    exit(1234);
                }
                vset(t1,i,t1v);
            }
            vset(logLambdaTilde,c,somatorio(t1) + constant + log(determinante(W[c])));
            vset(logPiTil,c,gsl_sf_psi(gsl_vector_get(alpha,c))-psiAlphaHat);
        }

        /*!computacao paralela de E - Eq 10.64*/

        arg1.K_ = arg2.K_ = K;
        arg1.D_ = arg2.D_ = D;
        arg1.s1_ = 0;
        arg2.s1_ = N>>1;
        arg1.s2_ = N>>1;
        arg2.s2_ = N;
        arg1.v_ = arg2.v_ = v;
        arg1.beta_ = arg2.beta_ = beta;
        arg1.logLambdaTilde_ = arg2.logLambdaTilde_ = logLambdaTilde;
        arg1.W_ = arg2.W_ = W;
        arg1.m_ = arg2.m_ = m;
        arg1.E_ = arg2.E_ = E;
        arg1.dado_ = arg2.dado_ = x;
        arg1.f = &f1;arg2.f = &f2;

        pthread_attr_init(&tattr1);
        pthread_attr_init(&tattr2);
        pthread_attr_setdetachstate(&tattr1,PTHREAD_CREATE_DETACHED);
        pthread_attr_setdetachstate(&tattr2,PTHREAD_CREATE_DETACHED);
        pthread_create(&tid1,&tattr1,Ecomp,&arg1);
        pthread_create(&tid2,&tattr2,Ecomp,&arg2);

        while(1)
            if (f1 && f2)
                break;
        f1 = f2 = 0;
        pthread_attr_destroy(&tattr1);
        pthread_attr_destroy(&tattr2);

        gsl_vector_set_zero(rhoexprow);
        gsl_vector_set_all(onesN,1);

        gsl_vector_add(rhoexprow,logLambdaTilde);
        gsl_vector_scale(rhoexprow,1/2);
        gsl_vector_add(rhoexprow,logPiTil);
        mKxN = matrix_from_vec_x_vec(rhoexprow,onesN,1,1,mKxN);
        gsl_matrix_transpose_memcpy(logRho,mKxN);
        gsl_matrix_scale(E,0.5);
        gsl_matrix_sub(logRho,E);

        gsl_vector_free(rhoexprow);

        rhoexprow = gsl_vector_alloc(N);
        gsl_vector_set_zero(rhoexprow);

        /*!Computacao paralela de r*/
        rARG_T rarg1,rarg2;

        rarg1.K_ = rarg2.K_ = K;
        rarg1.N_ = rarg2.N_ = N;
        rarg1.rhoexprow_ = rarg2.rhoexprow_ = rhoexprow;
        rarg1.logRho_ = rarg2.logRho_ = logRho;
        rarg1.logr_ = rarg2.logr_ = logr;
        rarg1.r_ = rarg2.r_ = r;
        rarg1.f = &f1;
        rarg2.f = &f2;
        rarg1.s1_ = 0;
        rarg2.s1_ = N>>1;
        rarg1.s2_ = N>>1;
        rarg2.s2_ = N;

        pthread_attr_init(&tattr1);
        pthread_attr_init(&tattr2);
        pthread_attr_setdetachstate(&tattr1,PTHREAD_CREATE_DETACHED);
        pthread_attr_setdetachstate(&tattr2,PTHREAD_CREATE_DETACHED);
        pthread_create(&tid1,&tattr1,rcomp,&rarg1);
        pthread_create(&tid2,&tattr2,rcomp,&rarg2);

        while(1)
            if(f1 && f2)
                break;
        f1 = f2 = 0;
        pthread_attr_destroy(&tattr1);
        pthread_attr_destroy(&tattr2);

        diff1 = gsl_matrix_alloc(D,N);
        diff2 = gsl_matrix_alloc(D,N);

        ///Eqs 10.83
        for(k=0; k<K; k++)
        {
            gsl_vector_view rk = gsl_matrix_column(r,k);
            vset(pi,k,somatorio(&(rk.vector))/N);
        }

        ///Eqs 10.51,52,53
        for(i=0; i<Nk->size; i++)
        {
            gsl_vector_view vis = gsl_matrix_column(r,i),rk;

            gsl_vector_memcpy(rhoexprow,&(vis.vector));

            vset(Nk,i,somatorio(rhoexprow)+0.0000000001);

            vis = gsl_matrix_column(xbar,i);
            rk = gsl_matrix_column(r,i);
            gsl_vector_set_all(onesD,1);
            matrix_from_vec_x_vec(&(rk.vector),onesD,1,1,mNxD);
            gsl_matrix_transpose_memcpy(rkq,mNxD);
            gsl_matrix_memcpy(diff2,rkq);
            gsl_matrix_mul_elements(rkq,x);

            for (j=0; j<xbar->size1; j++)
            {
                gsl_vector_view row = gsl_matrix_row(rkq,j);
                vset(&(vis.vector),j,somatorio(&(row.vector)) / vget(Nk,i));
            }

            matrix_from_vec_x_vec(&(vis.vector),onesN,1,1,diff1);
            gsl_matrix_sub(diff1,x);
            gsl_matrix_scale(diff1,-1.0);

            gsl_matrix_mul_elements(diff2,diff1);
            gsl_blas_dgemm(CblasNoTrans,CblasTrans,1,diff2,diff1,0,S[i]);
            //gsl_matrix_mul_elements(S[i],IDxD);
            gsl_matrix_scale(S[i],1/vget(Nk,i));
        }

        ///Calculo do limite inferior variacional
        logCalpha0 = gsl_sf_lngamma(K*alpha0) - K*gsl_sf_lngamma(alpha0);

        logB0 = (v0/2)*log(determinante(invW0)) - (v0*D/2)*log(2) - (D*(D-1)/4)*log(PI);
        for(i=0; i<D; i++)
            vset(onesD,i,gsl_sf_lngamma((v0-i)/2));
        logB0 -= somatorio(onesD);
        gsl_vector_set_all(onesD,1);

        alphgamln = gsl_vector_alloc(alpha->size);
        for(i=0; i<alpha->size; i++)
            gsl_vector_set(alphgamln,i,gsl_sf_lngamma(vget(alpha,i)));
        logCalpha = gsl_sf_lngamma(somatorio(alpha)) - somatorio(alphgamln);

        HqLambda = gsl_vector_alloc(K);

        vdiffD1 = gsl_vector_alloc(D);
        vdiffD2 = gsl_vector_alloc(D);
        for(k=0; k<K; k++)
        {
            double logBk = -(vget(v,k)/2)*log(determinante(W[k])) - (vget(v,k)*D/2)*log(2) - (D*(D-1)/4)*log(PI);
            double tmp;
            for(i=0; i<D; i++)
                vset(onesD,i,gsl_sf_lngamma((vget(v,k)-i)/2));
            logBk -= somatorio(onesD);

            vset(HqLambda, k, 0.5*vget(v,k)*D - logBk - 0.5*(vget(v,k)-D-1)*vget(logLambdaTilde,k));

            gsl_blas_dgemm(CblasNoTrans,CblasNoTrans,1,S[k],W[k],0,mDxD);
            //gsl_matrix_scale(mDxD,vget(v,k));
            vset(trSW,k,mtrace(mDxD));

            visu = gsl_matrix_column(xbar,k);
            gsl_vector_memcpy(vdiffD1,&(visu.vector));
            visu = gsl_matrix_column(m,k);
            gsl_vector_sub(vdiffD1,&(visu.vector));
            gsl_blas_dgemv(CblasNoTrans,1,W[k],vdiffD1,0,vdiffD2);
            gsl_blas_ddot(vdiffD1,vdiffD2,&tmp);
            vset(xbarWxbar,k,tmp);

            visu = gsl_matrix_column(m,k);
            gsl_vector_memcpy(vdiffD1,&(visu.vector));
            gsl_vector_sub(vdiffD1,m0);
            gsl_blas_dgemv(CblasNoTrans,1,W[k],vdiffD1,0,vdiffD2);
            gsl_blas_ddot(vdiffD1,vdiffD2,&tmp);
            vset(mWm,k,tmp);

            gsl_blas_dgemm(CblasNoTrans,CblasNoTrans,1,invW0,W[k],0,mDxD);
            vset(trW0invW,k,mtrace(mDxD));
        }

        ///Eqs 10.71,...,10.77
        for(i=0; i<K; i++)
            vset(alphgamln,i,vget(logLambdaTilde,i) - D/vget(beta,i) - vget(v,i)*vget(trSW,i) - vget(v,i)*vget(xbarWxbar,i) - D*log(2*PI));
        gsl_vector_mul(alphgamln,Nk);
        Lt1 = 0.5*somatorio(alphgamln);

        gsl_vector_memcpy(alphgamln,Nk);
        gsl_vector_mul(alphgamln,logPiTil);
        Lt2 = somatorio(alphgamln);

        Lt3 = logCalpha0 + (alpha0-1)*somatorio(logPiTil);

        DlogBeta0 = D*log(beta0/(2*PI));
        for(i=0; i<K; i++)
            vset(alphgamln,i,DlogBeta0 + vget(logLambdaTilde,i) - D*beta0/vget(beta,i) - beta0*vget(v,i)*vget(mWm,i));
        Lt41 = 0.5*somatorio(alphgamln);

        gsl_vector_mul(trW0invW,v);
        Lt42 = K*logB0 + 0.5*(v0-D-1)*somatorio(logLambdaTilde) - 0.5*somatorio(trW0invW);

        Lt4 = Lt41+Lt42;

        Lt5 = 0;
        for (i=0; i<r->size1; i++)
        {
            gsl_vector_view vv;
            vv = gsl_matrix_row(r,i);
            gsl_vector_memcpy(onesK,&(vv.vector));
            vv = gsl_matrix_row(logr,i);
            gsl_vector_mul(onesK,&(vv.vector));
            Lt5 += somatorio(onesK);
        }

        for(i=0; i<K; i++)
            vset(alphgamln,i,(vget(alpha,i)-1)*vget(logPiTil,i));
        Lt6 = somatorio(alphgamln) + logCalpha;

        for(i=0; i<K; i++)
            vset(alphgamln,i,0.5*vget(logLambdaTilde,i) + 0.5*D*log(vget(beta,i)/(2*PI)) - 0.5*D - vget(HqLambda,i));
        Lt7 = somatorio(alphgamln);

        vset(vbg->L,t,Lt1+Lt2+Lt3+Lt4-(Lt5+Lt6+Lt7));

        printf("%d: %.7f\n",t,vget(vbg->L,t));

        if(t > 1 && (vget(vbg->L,t-1) > vget(vbg->L,t)))
            printf("Alerta: Limite inferior decresceu: %f\n",vget(vbg->L,t-1) - vget(vbg->L,t));

        /*VM*/
        for(i=0; i<K; i++)
            vset(alpha,i,alpha0 + vget(Nk,i));
        for(i=0; i<K; i++)
            vset(beta,i,beta0 + vget(Nk,i));
        for(i=0; i<K; i++)
            vset(v,i,v0 + vget(Nk,i));

        /*novo m*/
        for(i=0; i<K; i++)
        {
            gsl_vector_view mi,xbari;
            mi = gsl_matrix_column(m,i);
            gsl_vector_memcpy(&(mi.vector),m0);
            gsl_vector_scale(&(mi.vector),beta0);

            xbari = gsl_matrix_column(xbar,i);
            gsl_vector_memcpy(onesD,&(xbari.vector));
            gsl_vector_scale(onesD,vget(Nk,i));

            gsl_vector_add(&(mi.vector),onesD);
            gsl_vector_scale(&(mi.vector),1/vget(beta,i));
        }

        /*novo W*/
        permD = gsl_permutation_alloc(D);
        for(k=0; k<K; k++)
        {
            double var1;
            gsl_vector_view visao = gsl_matrix_column(xbar,k);
            var1 = beta0*vget(Nk,k)/(beta0+vget(Nk,k));
            gsl_vector_memcpy(onesD,&(visao.vector));
            gsl_vector_sub(onesD,m0);
            mDxD = matrix_from_vec_x_vec(onesD,onesD,1,1,mDxD);
            gsl_matrix_scale(mDxD,var1);
            gsl_matrix_scale(S[k],vget(Nk,k));
            gsl_matrix_add(mDxD,S[k]);
            gsl_matrix_scale(S[k],1/vget(Nk,k));
            gsl_matrix_add(mDxD,invW0);
            gsl_linalg_LU_decomp (mDxD, permD, &i);
            gsl_linalg_LU_invert (mDxD, permD, W[k]);
            //gsl_matrix_mul_elements(W[k],IDxD);
        }

        if(t>0)
        {
            double actLower = vget(vbg->L,t);
            double antLower = vget(vbg->L,t-1);
            likIncr = ((actLower-antLower)/antLower);
            likIncr *= likIncr;
            likIncr = sqrt(likIncr);
        }

        gsl_matrix_free(diff1);
        gsl_matrix_free(diff2);
        gsl_matrix_free(rkq);
        gsl_matrix_free(mNxD);
        gsl_matrix_free(logRho);
        gsl_matrix_free(r);
        gsl_matrix_free(logr);
        gsl_matrix_free(mDxD);
        gsl_matrix_free(mKxN);

        gsl_vector_free(alphgamln);
        gsl_vector_free(logPiTil);
        gsl_vector_free(rhoexprow);
        gsl_vector_free(t1);
        gsl_permutation_free(permD);
        gsl_vector_free(vdiffD1);
        gsl_vector_free(vdiffD2);
        gsl_vector_free(HqLambda);

        ///Para se convergir ou decair
        if((likIncr < THRES) /*|| ((t > 1) && (vget(vbg->L,t) < vget(vbg->L,t-1)))*/)
            break;

        gsl_vector_set_all(onesK,1.0);
        gsl_vector_set_all(onesD,1.0);
        gsl_vector_set_all(onesN,1.0);
    }

    vbg->W = W;
    vbg->alpha = alpha;
    vbg->beta = beta;
    vbg->v = v;
    vbg->m = m;
    vbg->S = S;
    vbg->pi = pi;
    vbg->xbarra = xbar;

//    for(i=0; i<K; i++)
//        gsl_matrix_free(S[i]);
//
//    free(S);

    gsl_vector_free(trW0invW);
    gsl_vector_free(mWm);
    gsl_vector_free(xbarWxbar);
    gsl_vector_free(trSW);
    gsl_vector_free(logLambdaTilde);
    gsl_vector_free(onesD);
    gsl_vector_free(onesK);
    gsl_vector_free(onesN);
    gsl_vector_free(Nk);

    gsl_matrix_free(E);
    gsl_matrix_free(invW0);
    gsl_matrix_free(m2);
    gsl_matrix_free(x);
//    gsl_matrix_free(xbar);
    gsl_matrix_free(IDxD);

    return ;
}

void treinoEM(gmm* gmix, data *feas, workers *pool, int imax)
{
    number i,o,x=0;
    decimal last=INT_MIN,llh,sigma=0.01,m=-1.0;
    for(o=1; o<=imax; o++)
    {
        for(i=1; i<=imax; i++)
        {
            llh=gmm_EMtrain(feas,gmix,pool); /* Compute one iteration of EM algorithm.   */
            fprintf(stdout,"Iteration: %05i    Improvement: %3i%c    LogLikelihood: %.3f\n",
                    i,abs(round(-100*(llh-last)/last)),'%',llh); /* Show the EM results.  */
            if(last-llh>-sigma||isnan(last-llh))break; /* Break with sigma threshold. */
            last=llh;
        }
        x=gmix->num;
        if(m>=0)
        {
            gmix=gmm_merge(gmix,feas,m,pool);
            fprintf(stdout,"Number of Components: %06i\n",gmix->num);
        }
        if(x==gmix->num)break;
        last=INT_MIN;
    }
    //workers_finish(pool);
}
