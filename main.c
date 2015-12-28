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

double ALPHA0,BETA0,V0;
double W0IF;

int main2(int pK,int spk,char ftrn[30], const char *nomeexp)
{
    VBGMM *vbg;

    int numK;
    int i,j;

    workers *pool=NULL;

    data *dado = NULL;

    char ftname[150];
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

    /*! INICIALIZACAO DO VBGMM*/
    alpha0 = ALPHA0;
    beta0 = BETA0;
    v0 = V0;

    gm = gmm_initialize(dado,vbg->K);
    treinoEM(gm,dado,pool,15);

    gsl_matrix_set_identity(W0);
    gsl_matrix_scale(W0,W0IF);
    gsl_vector_set_all(m0,0.0);
    if (W0IF < 0)
        for (numK=0; numK<vbg->dim; numK++)
            mset(W0,numK,numK,dado->variance[numK]);
    for (numK=0; numK<vbg->dim; numK++)
        vset(m0,numK,dado->mean[numK]);
    if (alpha0 < 0)
        alpha0 = ((double)dado->samples/vbg->K);
    /// FIM INICIALIZACAO

    printf("Variational EM\n");
    vem_train(vbg,gm,dado,alpha0,beta0,v0,m0,W0);

    saveVGMM("spker.txt",vbg);

    printf("VEM Ok.\nExecutando Testes...\n");

    /*!testes*/
    pscores = gsl_vector_alloc(54);
    nscores = gsl_vector_alloc(54*40);
    for (i=0;i<54;i++)
    {
        sprintf(ftname,"testes/teste%d/teste%d_%d.txt",spk,spk,i+1);
        vset(pscores,i,runtest(ftname,vbg,spk));
    }

    sprintf(ftname,"testes/scrs/%s/K%d/scores_pos_%d.bin",nomeexp,vbg->K,spk);
    savescore(ftname,pscores,vbg->K);

    for (i=0;i<40;i++)
        for (j=0;j<54;j++)
        {
            sprintf(ftname,"testes/imposter/imposter%d_%d.txt",i+1,j+1);
            vset(nscores,i*54+j,runtest(ftname,vbg,spk));
        }

    sprintf(ftname,"testes/scrs/%s/K%d/scores_neg_%d.bin",nomeexp,vbg->K,spk);
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

    if (argc == 6)
    {
        ALPHA0 = atof(argv[1]);
        BETA0 = atof(argv[2]);
        V0 = atof(argv[3]);
        W0IF = atoi(argv[4]);
        sprintf(exp,"%s",argv[5]);
    }
    if (argc == 4)
    {
        plotscr(argv[2],atoi(argv[3]));
        exit(0);
    }
    if (argc == 1)
    {
        printf("SRE <a0> <beta0> <v0> <W0_SCALE> <EXP_NAME>\na0: ");
        scanf("%lf",&ALPHA0);
        printf("b0: ");scanf("%lf",&BETA0);
        printf("v0: ");scanf("%lf",&V0);
        printf("W0_SCALE: ");scanf("%lf",&W0IF);
        printf("exp_name: ");scanf("%s",exp);
        sprintf(exp,"%s","exp1");
    }

    printf("a0: %lf\nbeta0: %lf\nv0: %lf\nW0_SCALE: %lf\nexp_name: %s",ALPHA0,BETA0,V0,W0IF,exp);
    printf("\nok (y/n)? ");
    scanf("%c",cmd+1);

    if (*(cmd+1) == 'n')
        exit(0);

    heap_MTX = gsl_matrix_alloc(DIM,DIM);
    heap_VD = gsl_vector_alloc(DIM);
    mtxd2 = gsl_matrix_alloc(DIM,2);
    tmpscore = gsl_matrix_alloc(DIM,DIM);

    permutglobal = gsl_permutation_alloc(DIM);

    //main2(6,0,"faithful.txt","nn");

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
        sprintf(treino,"treino/treino%d.txt",i);
        main2(8,i,treino,exp);
        main2(16,i,treino,exp);
        main2(32,i,treino,exp);
        main2(64,i,treino,exp);
    }
    gsl_matrix_free(heap_MTX);
    gsl_matrix_free(mtxd2);
    gsl_matrix_free(tmpscore);
    gsl_vector_free(heap_VD);
    gsl_permutation_free(permutglobal);
    return 0;
}

void plotscr (char *exp, int k)
{
    char pathpos[100],pathneg[100];
    double pos[54],neg[2160];
    FILE *posf = NULL, *negf = NULL;
    sprintf(pathpos,"testes/scrs/%s/K%d/scores_pos_1.bin",exp,k);
    sprintf(pathneg,"testes/scrs/%s/K%d/scores_neg_1.bin",exp,k);

    posf = fopen(pathpos,"rb");
    negf = fopen(pathneg,"rb");

    if(!posf || !negf)
    {
        printf("Arquivos não econtrados\n%s\n%s\n",pathpos,pathneg);
        exit(1);
    }

    fread(pos,sizeof(double),54,posf);
    fread(neg,sizeof(double),2160,negf);

    fclose(posf);
    fclose(negf);
}
