
#include "vem.h"

void f_del (data *d)
{
    int i=0;
    for (i=0;i<d->samples;i++)
        free(d->data[i]);
    free(d->data);
    free(d->mean);
    free(d->variance);
    free(d);
}

data *f_load (char *nome)
{
    FILE *f = fopen(nome,"rb");
    int n,d,i,j;
    data *r;
    if (!f)
    {
        printf("Nenhum arquivo: %s encontrado\n",nome);
        exit(9);
    }
    fscanf(f,"%d %d",&d,&n);
    r = (data*) malloc(sizeof(data));
    r->dimension = d;
    r->samples = n;
    r->data = (double**) malloc(n*sizeof(double*));
    for (i=0;i<n;i++)
        r->data[i] = (double*) malloc(d*sizeof(double));
    for (i=0;i<n;i++)
        for(j=0;j<d;j++)
            fscanf(f,"%lf ",r->data[i]+j);
    r->mean = (double*) malloc(1*sizeof(double));
    r->variance = (double*) malloc(1*sizeof(double));
    fclose(f);
    return r;
}

double runtest (char *fname,VBGMM *modelo, int spk)
{
    gsl_matrix *fala;
    int i,j;
    double scr;
    data *teste = NULL;

    teste = f_load(fname);

    fala = gsl_matrix_alloc(teste->samples,teste->dimension);

    //printf("spk = %d K = %d, %s\n",spk,modelo->K,fname);

    for(i=0;i<teste->samples;i++)
        for(j=0;j<teste->dimension;j++)
            mset(fala,i,j,teste->data[i][j]);

    scr = score(fala,modelo);
    //printf("%.1f\n",scr);

    f_del(teste);
    gsl_matrix_free(fala);
    return scr;
}

void savescore (char *fname, gsl_vector *vt, int K)
{
    FILE *fl;
    fl = fopen(fname,"w");
    gsl_vector_fprintf(fl,vt,"%.3f");
    fclose(fl);
}
