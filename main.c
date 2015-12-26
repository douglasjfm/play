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

#include <stdio.h>

#include "vem.h"

gsl_vector *heap_VD;
gsl_matrix *heap_MTX;

gsl_matrix *mtxd2;
gsl_matrix *tmpscore;

gsl_permutation *permutglobal;

int main2(int pK,int spk,char ftrn[30], const char *nomeexp)
{
    VBGMM *vbg;
    int numK;
    workers *pool=NULL;
    data *dado = NULL;

    numK = pK;
    pool = workers_create(1);
    dado = feas_load(ftrn,pool);

    printf("K = %d N = %d dim = %d\n",numK,dado->samples,dado->dimension);

    vbg = VBGMM_alloc(numK,dado->dimension);

    /*! INICIALIZACAO DO VBGMM*/
    gsl_vector *m0 = heap_VD;
    gsl_matrix *W0 = heap_MTX;
    gmm *gm;
    double alpha0,beta0,v0;
    alpha0 = ((double)dado->samples/vbg->K);
    beta0 = 1.0;
    v0 = vbg->dim;

    gm = gmm_initialize(dado,vbg->K);
    treinoEM(gm,dado,pool,5);

    gsl_matrix_set_identity(W0);
    //gsl_matrix_scale(W0,1.0);
    gsl_vector_set_all(m0,0.0);

    //for (numK=0; numK<vbg->dim; numK++)
        //mset(W0,numK,numK,dado->variance[numK]);
    /// FIM INICIALIZACAO

    printf("Variational EM\n");
    vem_train(vbg,gm,dado,alpha0,beta0,v0,m0,W0);

    printf("VEM Ok.\nExecutando Testes...\n");

    /*!testes*/
    char ftname[150];
    gsl_vector *pscores = gsl_vector_alloc(54),*nscores = gsl_vector_alloc(54*40);
    int i,j;
    for (i=0;i<54;i++)
    {
        sprintf(ftname,"testes/teste%d/teste%d_%d.txt",spk,spk,i+1);
        vset(pscores,i,runtest(ftname,vbg,spk));
    }
    sprintf(ftname,"testes/scrs/%s/K%d/scores_pos_%d.bin",nomeexp,vbg->K,spk);
    savescore(ftname,pscores,vbg->K);
    gsl_vector_free(pscores);
    for (i=0;i<40;i++)
        for (j=0;j<54;j++)
        {
            sprintf(ftname,"testes/imposter/imposter%d_%d.txt",i+1,j+1);
            vset(nscores,i*54+j,runtest(ftname,vbg,spk));
        }
    sprintf(ftname,"testes/scrs/%s/K%d/scores_neg_%d.bin",nomeexp,vbg->K,spk);
    savescore(ftname,nscores,vbg->K);
    gsl_vector_free(nscores);

    feas_delete(dado);
    gmm_delete(gm);
    vbg_delete(vbg);
    return 0;
}

int main(int argc, char *argv[])
{
    int i;
    char exp[30];
    char cmd[50];

    sprintf(exp,"%s","exp1");

    heap_MTX = gsl_matrix_alloc(57,57);
    heap_VD = gsl_vector_alloc(57);
    mtxd2 = gsl_matrix_alloc(57,2);
    tmpscore = gsl_matrix_alloc(57,57);

    permutglobal = gsl_permutation_alloc(57);

    sprintf(cmd,"mkdir testes/scrs/%s/K8",exp);
    system(cmd);
    sprintf(cmd,"mkdir testes/scrs/%s/K16",exp);
    system(cmd);
    sprintf(cmd,"mkdir testes/scrs/%s/K32",exp);
    system(cmd);
    sprintf(cmd,"mkdir testes/scrs/%s/K64",exp);
    system(cmd);
    sprintf(cmd,"mkdir testes/scrs/%s/K128",exp);
    system(cmd);

    for (i=1;i<=48;i++)
    {
        char treino[30];
        sprintf(treino,"treino/treino%d.txt",i);
        main2(8,i,treino,exp);
        main2(16,i,treino,exp);
        main2(32,i,treino,exp);
        main2(64,i,treino,exp);
        main2(128,i,treino,exp);
    }
    gsl_matrix_free(heap_MTX);
    gsl_matrix_free(mtxd2);
    gsl_matrix_free(tmpscore);
    gsl_vector_free(heap_VD);
    gsl_permutation_free(permutglobal);
    return 0;
}
