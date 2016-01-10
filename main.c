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
gsl_matrix *tmpscore2;
gsl_matrix *tmpscore3;

gsl_permutation *permutglobal;

double ALPHA0,BETA0,V0;
double W0IF,M0IF;

unsigned int DIM;

char modo;

int main2(int pK,int spk,char ftrn[30], const char *nomeexp)
{
    VBGMM *vbg;

    int numK;
    int i,j,c=0;

    workers *pool=NULL;

    data *dado = NULL;

    char ftname[150], cmd[100];
    gsl_vector *pscores, *nscores;

    gsl_vector *m0 = heap_VD;
    gsl_matrix *W0 = heap_MTX;

    gmm *gm;

    double alpha0,beta0,v0;

    numK = pK;
    pool = workers_create(1);
    dado = feas_load(ftrn,pool);

    printf("K = %d N = %d dim = %d\n",numK,dado->samples,dado->dimension);

    vbg = VBGMM_alloc(numK,dado->dimension);

    /*!INICIALIZACAO DO VBGMM*/
    alpha0 = ALPHA0;
    beta0 = BETA0;
    v0 = V0;

    gm = gmm_initialize(dado,vbg->K);
    treinoEM(gm,dado,pool,5);
    ///Inicializa W0 com a matriz id ou Precisao dos dados ou Min Var.
    gsl_matrix_set_identity(W0);
    if (W0IF < -10)
        for (j=0; j<vbg->dim; j++)
            mset(W0,j,j,1.0/dado->variance[j]);
    else if (W0IF < 0) ///Inicializa com a covariancia minima do EM
        for (j=0; j<vbg->dim; j++)
            mset(W0,j,j,gm->mcov[j]);
    else
        gsl_matrix_scale(W0,W0IF);

    ///Inicializa m0 com a media dos dados ou zero.
    gsl_vector_set_all(m0,0.0);
    if (M0IF < 0)
        for (j=0; j<vbg->dim; j++)
            vset(m0,j,dado->mean[j]);
    ///Inicializa alfa0
    if (alpha0 < 0)
        alpha0 = abs(alpha0 * dado->samples);
    /// FIM INICIALIZACAO

    printf("Variational EM\n");
    vem_train(vbg,gm,dado,alpha0,beta0,v0,m0,W0);

    sprintf(ftname,"res%s/modelo/spkr%d_%d.txt",nomeexp,spk,pK);
    saveVGMM(ftname,vbg);

    printf("VEM Ok.\nExecutando Testes...locutor: %d\n",spk);

    /*!testes*/
    pscores = gsl_vector_alloc(54);
    nscores = gsl_vector_alloc(54*40);

    sprintf(cmd,"mkdir res%s/scrs/K%d",nomeexp,vbg->K);
    system(cmd);

    calcDetL(vbg,modo);
    if (modo=='n') calcInvS(vbg);

    printf("\n-----------------------------------------");
    fflush(stdout);

    for (i=0;i<54;i++)
    {
        sprintf(ftname,"testes%s/teste%d/teste%d_%d.txt",nomeexp,spk,spk,i+1);
        vset(pscores,i,runtest(ftname,vbg,spk,modo));
        if(i==53)
        {
            printf("%c%c%c",8,32,8);
            fflush(stdout);
        }
    }

    sprintf(ftname,"res%s/scrs/K%d/scores_pos_%d.bin",nomeexp,vbg->K,spk);
    savescore(ftname,pscores,vbg->K);

    for (i=0;i<40;i++)
        for (j=0;j<54;j++)
        {
            sprintf(ftname,"testes%s/imposter/imposter%d_%d.txt",nomeexp,i+1,j+1);
            vset(nscores,i*54+j,runtest(ftname,vbg,spk,modo));
            if(j==53)
            {
                printf("%c%c%c",8,32,8);
                fflush(stdout);
            }
        }
    printf("\n");
    sprintf(ftname,"res%s/scrs/K%d/scores_neg_%d.bin",nomeexp,vbg->K,spk);
    savescore(ftname,nscores,vbg->K);

    workers_finish(pool);
    gsl_vector_free(nscores);
    gsl_vector_free(pscores);
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
    char treino[30];

    modo = 'n';

    //printf("%lf",gsl_sf_psi((39.22 - 0)/2)),exit(1);

    if (argc == 7)
    {
        ALPHA0 = atof(argv[1]);
        BETA0 = atof(argv[2]);
        V0 = atof(argv[3]);
        W0IF = atof(argv[4]);
        M0IF = atof(argv[5]);
        sprintf(exp,"%s",argv[6]);
    }

    else if (argc == 1)
    {
        printf("SRE <a0> <beta0> <v0> <W0_SCALE> <M0_COND> <EXP_NAME>\na0: ");
        scanf("%lf",&ALPHA0);
        printf("b0: ");scanf("%lf",&BETA0);
        printf("v0: ");scanf("%lf",&V0);
        printf("W0_SCALE: ");scanf("%lf",&W0IF);
        printf("M0_COND: ");scanf("%lf",&M0IF);
        printf("exp_name: ");scanf("%s",exp);
    }
    else printf("Numero invalido de parametros\n"),exit(0xE);
    printf("Dimensionalidade: ");
    scanf("%u",&DIM);

    printf("a0: %lf\nbeta0: %lf\nv0: %lf\nW0_SCALE: %lf\nM0_COND: %lf\nexp_name: %s\nDIM: %u\n",ALPHA0,BETA0,V0,W0IF,M0IF,exp,DIM);
    printf("\nok (y/n)? ");
    scanf("%s",cmd+1);

    if (*(cmd+1) != 'y')
        printf("'%c'",*(cmd+1)),exit(0);

    heap_MTX = gsl_matrix_alloc(DIM,DIM);
    heap_VD = gsl_vector_alloc(DIM);
    mtxd2 = gsl_matrix_alloc(DIM,2);
    tmpscore = gsl_matrix_alloc(DIM,DIM);
    tmpscore2 = gsl_matrix_alloc(DIM,DIM);
    tmpscore3 = gsl_matrix_alloc(DIM,DIM);

    permutglobal = gsl_permutation_alloc(DIM);

    sprintf(cmd,"mkdir res%s",exp);
    system(cmd);
    sprintf(cmd,"mkdir res%s/modelo",exp);
    system(cmd);
    sprintf(cmd,"mkdir res%s/scrs",exp);
    system(cmd);

    for (i=1;i<=48;i++)
    {
        sprintf(treino,"treino%s/treino%d.txt",exp,i);
        main2(8,i,treino,exp);
        main2(16,i,treino,exp);
        main2(32,i,treino,exp);
        //main2(57,i,treino,exp);
        main2(64,i,treino,exp);
        //main2(80,i,treino,exp);
    }
    gsl_matrix_free(heap_MTX);
    gsl_matrix_free(mtxd2);
    gsl_matrix_free(tmpscore);
    gsl_matrix_free(tmpscore2);
    gsl_matrix_free(tmpscore3);
    gsl_vector_free(heap_VD);
    gsl_permutation_free(permutglobal);
    return 0;
}
