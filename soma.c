//SOMA implementation, mostly based on code by Ivan Zelinka 

#include "soma.h"
#include "math.h"
#include "lib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static double *Population,*TMPPopulation,*PRTV,*History,*Specimen;
static int PD,PNP,k,kk,l,n,neval,i,j,NP,PopSize,D,Migrations,smc2,V1,V2,V3,Generations;
static double h,hh,ckl1,PathLength,Step,PRT,MinDiv,F,CR,K;
static char	InFile[12],OutFile[12],HstFile[12];
static FILE *in;

static void fill_hist_vector (SOMAreturn *sreturn, double sstot);
static void CostValue(int Individual, Array2D *soma_array, MuExpr *soma_muexpr, double sstot);
static void CostValue2(int Individual, FFparam *ffp, Array2D *soma_array, FitnessFunction fitfunc, double sstot);
static double calc_sstot (Array2D *array, int column);

#define RANDOM_V ((double)rand()/(double)RAND_MAX)

int SOMA (SOMAsetup *ssetup, int numparams, Parameter *paramlist, SOMAreturn *somaret, Array2D *array, MuExpr *muexpr)
{
    double sstot;

    srand (time(NULL));
    /*   clrscr();
         if (argc == 1)
         {
         printf("Missing configuration file in command line!!!\n");
         goto END;
         }*/

    //sstot does not depend on parameters
    sstot=calc_sstot (array,0);


    strcpy(InFile,"report");
    strcpy(OutFile,"report");
    strcpy(HstFile,"report");
    strcat(InFile,".txt");
    strcat(OutFile,".out");
    strcat(HstFile,".hst");
    /*   if ((in = fopen(InFile, "rt")) == NULL)
         {
         printf("Cannot open configuration file !!!\n");
         goto END;
         }

         fscanf(in, "%lf", &PathLength);fgetc(in);
         fscanf(in, "%lf", &Step);fgetc(in);
         fscanf(in, "%lf", &PRT);fgetc(in);
         fscanf(in, "%d",&PopSize);fgetc(in);
         fscanf(in, "%d",&D);fgetc(in);
         fscanf(in, "%d",&Migrations);fgetc(in);
         fscanf(in, "%lf",&MinDiv); */

    PathLength=ssetup->PathLength;
    Step=ssetup->Step;
    PRT=ssetup->PRT;    //perturbation chance
    PopSize=ssetup->PopSize;
    Migrations=ssetup->Migrations;
    MinDiv=ssetup->MinDiv;
    D=numparams;

    createPopulation(D+1, PopSize+3);
    createPRTV(D);
    createHistory(Migrations);
    createSpecimen(D);

    k=neval=0;
    for(i=0;i<PD-1;i++) {
        j=0;
        //for(j=0;j<3;j++)
        //	{
        //fscanf(in, "%lf", &ckl1);fgetc(in);
        //	setSpecimen(j,i,ckl1);
        //	k++;
        //	}
        setSpecimen (j++, i, paramlist[i].min);
        setSpecimen (j++, i, paramlist[i].max);
        setSpecimen (j++, i, 0);
    }
    //   fclose(in);

    //initialize population
    //randomize();
    for(i=0;i<PNP-3;i++)
    {
        for(j=0;j<PD-1;j++)
        {
            setPopulation(j, i, getSpecimen(0,j)+RANDOM_V*(getSpecimen(1,j)-getSpecimen(0,j)));
        }
        neval++;
        CostValue(i, array, muexpr, sstot);
    }

    //search for the best and the worst of individuals
    WorstIndividual();
    BestIndividual();
    fill_hist_vector(somaret, sstot);
    for(l=0;l<Migrations;l++)
    {
        //clrscr();
        //printf("Migration loops = %d\nBest individual = %lg\nWorst individual = %lg",l,h,hh);
        //append to History
        History[l]=h;
        if(MinDiv>=fabs(fabs(h)-fabs(hh)))
            goto SKIP;

        //Migration
        for(i=0;i<PNP-3;i++)
        {
            if(i!=k)
            {
                for(n=0;n<PD;n++)
                    setPopulation(n,PNP-2,getPopulation(n,i));
                //Original position of PRTVector
                for(n=0;n<PD-1;n++)
                {
                    if(RANDOM_V>PRT)
                        PRTV[n]=0.0;
                    else
                        PRTV[n]=1.0;
                }
                for(ckl1=0;ckl1<=PathLength;ckl1=ckl1+Step)
                {
                    //PRTVector here increase speed of convergence
                    for(n=0;n<PD-1;n++)
                    {
                        if(RANDOM_V>PRT)
                            PRTV[n]=0.0;
                        else
                            PRTV[n]=1.0;
                    }

                    for(j=0;j<PD-1;j++)
                    {
                        setPopulation(j,PNP-3,getPopulation(j,i)+(getPopulation(j,k)-getPopulation(j,i))*ckl1*PRTV[j]);
                        //type of variable and boundary checking
                        if(getSpecimen(0,j)>getPopulation(j,PNP-3) || getSpecimen(1,j)<getPopulation(j,PNP-3))
                            setPopulation(j,PNP-3,getSpecimen(0,j)+RANDOM_V*(getSpecimen(1,j)-getSpecimen(0,j)));
                    }
                    neval++;
                    CostValue(PNP-3, array, muexpr, sstot);
                    if(getPopulation(PD-1,PNP-3)<getPopulation(PD-1,PNP-2))
                    {
                        for(n=0;n<PD;n++)
                            setPopulation(n,PNP-2,getPopulation(n,PNP-3));
                    }
                }
                for(n=0;n<PD;n++)
                    setPopulation(n,i,getPopulation(n,PNP-2));
            }
        }

        WorstIndividual();
        BestIndividual();
        somaret->solution++;
        fill_hist_vector (somaret, sstot);
        if (somaret->history[somaret->solution].Rsquared>=ssetup->PRV)
            break;
    }
SKIP:
    //ShowInfo("SOMA, version 'All To One'");
END:
    return 0;
}

int SOMA2 (SOMAsetup *ssetup, int numparams, Parameter *paramlist, SOMAreturn *somaret, Array2D *soma_array, FitnessFunction ffunc)
{
    int ind;
    FFparam* pffparam;
    double sstot;
    
    srand (time(NULL));

    /*   clrscr();
         if (argc == 1)
         {
         printf("Missing configuration file in command line!!!\n");
         goto END;
         }*/

    sstot=calc_sstot (soma_array, 1);

    strcpy(InFile,"report");
    strcpy(OutFile,"report");
    strcpy(HstFile,"report");
    strcat(InFile,".txt");
    strcat(OutFile,".out");
    strcat(HstFile,".hst");
    /*   if ((in = fopen(InFile, "rt")) == NULL)
         {
         printf("Cannot open configuration file !!!\n");
         goto END;
         }

         fscanf(in, "%lf", &PathLength);fgetc(in);
         fscanf(in, "%lf", &Step);fgetc(in);
         fscanf(in, "%lf", &PRT);fgetc(in);
         fscanf(in, "%d",&PopSize);fgetc(in);
         fscanf(in, "%d",&D);fgetc(in);
         fscanf(in, "%d",&Migrations);fgetc(in);
         fscanf(in, "%lf",&MinDiv); */

    PathLength=ssetup->PathLength;
    Step=ssetup->Step;
    PRT=ssetup->PRT;    //perturbation chance
    PopSize=ssetup->PopSize;
    Migrations=ssetup->Migrations;
    MinDiv=ssetup->MinDiv;
    D=numparams;

    //FF param initialization
    pffparam=(FFparam *) malloc (numparams*sizeof (FFparam));
    for (ind=0; ind<numparams; ind++) 
        strcpy (pffparam[ind].name , paramlist[ind].name);

    createPopulation(D+1, PopSize+3);
    createPRTV(D);
    createHistory(Migrations);
    createSpecimen(D);

    k=neval=0;
    for(i=0;i<PD-1;i++) {
        j=0;
        //for(j=0;j<3;j++)
        //	{
        //fscanf(in, "%lf", &ckl1);fgetc(in);
        //	setSpecimen(j,i,ckl1);
        //	k++;
        //	}
        setSpecimen (j++, i, paramlist[i].min);
        setSpecimen (j++, i, paramlist[i].max);
        setSpecimen (j++, i, 0);
    }
    //   fclose(in);

    //initialize population
    //randomize();
    for(i=0;i<PNP-3;i++)
    {
        for(j=0;j<PD-1;j++)
        {
            setPopulation(j, i, getSpecimen(0,j)+RANDOM_V*(getSpecimen(1,j)-getSpecimen(0,j)));
        }
        neval++;
        CostValue2(i, pffparam, soma_array, ffunc, sstot);
    }

    //search for the best and the worst of individuals
    WorstIndividual();
    BestIndividual();
    fill_hist_vector(somaret, sstot);
    for(l=0;l<Migrations;l++)
    {
        //clrscr();
        //printf("Migration loops = %d\nBest individual = %lg\nWorst individual = %lg",l,h,hh);
        //append to History
        History[l]=h;
        if(MinDiv>=fabs(fabs(h)-fabs(hh)))
            goto SKIP;

        //Migration
        for(i=0;i<PNP-3;i++)
        {
            if(i!=k)
            {
                for(n=0;n<PD;n++)
                    setPopulation(n,PNP-2,getPopulation(n,i));
                //Original position of PRTVector
                for(n=0;n<PD-1;n++)
                {
                    if(RANDOM_V>PRT)
                        PRTV[n]=0.0;
                    else
                        PRTV[n]=1.0;
                }
                for(ckl1=0;ckl1<=PathLength;ckl1=ckl1+Step)
                {
                    //PRTVector here increase speed of convergence
                    for(n=0;n<PD-1;n++)
                    {
                        if(RANDOM_V>PRT)
                            PRTV[n]=0.0;
                        else
                            PRTV[n]=1.0;
                    }

                    for(j=0;j<PD-1;j++)
                    {
                        setPopulation(j,PNP-3,getPopulation(j,i)+(getPopulation(j,k)-getPopulation(j,i))*ckl1*PRTV[j]);
                        //type of variable and boundary checking
                        if(getSpecimen(0,j)>getPopulation(j,PNP-3) || getSpecimen(1,j)<getPopulation(j,PNP-3))
                            setPopulation(j,PNP-3,getSpecimen(0,j)+RANDOM_V*(getSpecimen(1,j)-getSpecimen(0,j)));
                    }
                    neval++;
                    CostValue2(PNP-3, pffparam, soma_array, ffunc, sstot);
                    if(getPopulation(PD-1,PNP-3)<getPopulation(PD-1,PNP-2))
                    {
                        for(n=0;n<PD;n++)
                            setPopulation(n,PNP-2,getPopulation(n,PNP-3));
                    }
                }
                for(n=0;n<PD;n++)
                    setPopulation(n,i,getPopulation(n,PNP-2));
            }
        }

        WorstIndividual();
        BestIndividual();
        somaret->solution++;
        fill_hist_vector (somaret, sstot);
        if (somaret->history[somaret->solution].Rsquared>=ssetup->PRV)
            break;
    }
    free (pffparam);
SKIP:
    //ShowInfo("SOMA, version 'All To One'");
END:
    return 0;
}

void fill_hist_vector (SOMAreturn *sreturn, double sstot)
{
    int fill_i=0;
    for(fill_i=0;fill_i<PD-1;fill_i++)
    {
        if(getSpecimen(2,fill_i)==1)
        {
            sreturn->history[sreturn->solution].params[fill_i]=getIntPopulation(fill_i,k);
        }
        else
        {
            sreturn->history[sreturn->solution].params[fill_i]=getPopulation(fill_i,k);
        }
    }
    sreturn->history[sreturn->solution].Rsquared=1-h/sstot;   //Rsquared=1-ssres/sstot
}

int createPopulation(int pd, int pnp)
{

    PD = pd;
    PNP = pnp;

    Population = (double*) calloc(PNP * PD, sizeof(double));
    if (Population==NULL)
    {
        puts("Not enough memory !!!");
        exit(0);
    }

    return Population ? 0 : 1;
}

int destroyPopulation()
{
    PD = 0;
    PNP = 0;

    free(Population);
    Population = 0;

    return 0;
}

int createTMPPopulation()
{
    TMPPopulation = (double*) calloc(PNP * PD, sizeof(double));
    if (TMPPopulation==NULL)
    {
        puts("Not enough memory !!!");
        exit(0);
    }

    return TMPPopulation ? 0 : 1;
}

int destroyTMPPopulation()
{
    free(TMPPopulation);
    TMPPopulation = 0;

    return 0;
}

double getPopulation(int pd, int pnp)
{
    return Population[PD * pnp + pd];
}

double getIntPopulation(int pd, int pnp)
{
    return floor(Population[PD * pnp + pd]);
}


void setPopulation(int pd, int pnp, double pop)
{
    Population[PD * pnp + pd] = pop;
}

double getTMPPopulation(int pd, int pnp)
{
    return TMPPopulation[PD * pnp + pd];
}

void setTMPPopulation(int pd, int pnp, double pop)
{
    TMPPopulation[PD * pnp + pd] = pop;
}

int createSpecimen(int pd)
{
    Specimen = (double*) calloc(pd * 3, sizeof(double));
    if (Specimen==NULL)
    {
        puts("Not enough memory !!!");
        exit(0);
    }

    return Specimen ? 0 : 1;
}

int destroySpecimen()
{
    free(Specimen);
    Specimen = 0;

    return 0;
}

double getSpecimen(int dm, int pd)
{
    return Specimen[3 * pd + dm];
}

void setSpecimen(int dm, int pd, double pop)
{
    Specimen[3 * pd + dm] = pop;
}

int createPRTV(int pd)
{
    PRTV = (double*) calloc(pd, sizeof(double));
    if (PRTV==NULL)
    {
        puts("Not enough memory !!!");
        exit(0);
    }
    return Population ? 0 : 1;
}

int destroyPRTV()
{
    free(PRTV);
    PRTV = 0;
    return 0;
}

int createHistory(int ml)
{
    History = (double*) calloc(ml, sizeof(double));
    if (History==NULL)
    {
        puts("Not enough memory !!!");
        exit(0);
    }
    return Population ? 0 : 1;
}

int destroyHistory()
{
    free(History);
    History = 0;
    return 0;
}

void WorstIndividual()
{
    int	smc;
    kk=0;
    hh=getPopulation(PD-1,0);
    for(smc=1;smc<PNP-3;smc++)
        if(getPopulation(PD-1,smc)>hh)
        {
            kk=smc;
            hh=getPopulation(PD-1,smc);
        }
}

void BestIndividual()
{
    int	smc;
    k=0;
    h=getPopulation(PD-1,0);
    for(smc=1;smc<PNP-3;smc++)
        if(getPopulation(PD-1,smc)<h)
        {
            k=smc;
            h=getPopulation(PD-1,smc);
        }
}

void ShowInfo(const char *ret)
{
    //on screen
    //clrscr();
    //return the best individual
    printf("Calculated by %s\n\n",ret);
    printf("Evolution conditions\nPathLength = %f,\tStep = %f,\nPRT = %f,\t\tPopSize = %d,\nD = %d,\t\t\tMigrations = %d,\nMinDiv = %f\n", PathLength, Step, PRT,PopSize, D, Migrations, MinDiv);

    //cost value of the best individual
    ckl1=getPopulation(PD-1,k);
    printf("\nCost value of the best individual  = %lg\n", getPopulation(PD-1,k));
    ckl1=getPopulation(PD-1,kk);
    printf("Cost value of the worst individual = %lg\n", ckl1);
    printf("\nMigrations = %d\n", l);
    printf("Number of function evaluations = %d\n\n>>>> PLEASE SEE FILE SOMAOUT.TXT or DEOUT.TXT <<<<\n\n", neval);

    //to file
    if ((in = fopen(OutFile, "w")) == NULL)
    {
        printf("Cannot open output file %s !!!\n",OutFile);
        goto END;
    }
    //return the best individual
    fprintf(in,"Calculated by %s\n\n",ret);
    fprintf(in,"Evolution conditions\nPathLength = %f,\tStep = %f,\nPRT = %f,\t\tPopSize = %d,\nD = %d,\t\t\tMigrations = %d,\nMinDiv = %f\n", PathLength, Step, PRT,PopSize, D, Migrations, MinDiv);
    fprintf(in,"\nBest individual\n");
    for(i=0;i<PD-1;i++)
    {
        //      ckl1=getPopulation(i,k);
        //		fprintf(in,"Parameter %d = %lg\n", i+1, ckl1);
        if(getSpecimen(2,i)==1)
        {
            ckl1=getIntPopulation(i,k);
            fprintf(in,"Parameter %d = %lg\n", i+1, ckl1);
        }
        else
        {
            ckl1=getPopulation(i,k);
            fprintf(in,"Parameter %d = %lg\n", i+1, ckl1);
        }
    }

    //cost value of the best individual
    ckl1=getPopulation(PD-1,k);
    fprintf(in,"Best individual cost value = %lg\n", ckl1);
    fprintf(in,"Migrations = %d\n", l);
    fprintf(in,"Number of function evaluations = %d\n\n", neval);
    fprintf(in,
            "****************** Appendix ******************\n"
            "\nPLEASE DO NOT CHANGE TEXT IN ANY SOURCE CODE\n"
            "Updated by creator SOMA - Ivan Zelinka, 20.4.2001\n\n"
            "Source information:\n"
            "\tIf you are interested in more detail in SOMA or differential evolution\n"
            "\tthen please see:\n"
            "\thttp://www.ft.utb.cz/people/zelinka/soma.htm\n"
            "\tor contact :\n"
            "\t\tIvan Zelinka - zelinka@ft.utb.cz\n"
            "\t\tInstitute of Information Technologies, Faculty of Technology,\n"
            "\t\tThomas Bata University, Mostni 5139, 760 01 Zlin, Czech Republic\n\n"
            "\t\tJouni Lampinen - lampine@lut.fi\n"
            "\t\tLappeenranta University of Technology, Department of Information Technology,\n"
            "\t\tLaboratory of Information Processing, P.O.Box 20, FIN-53851 Lappeenranta, Finland\n\n"
            "How to handle this software:\n"
            "\tStructure of SOMAIN.TXT file is following (with recommended values):\n"
            "\t\tPathLength\t\t<0.1,5 >\n"
            "\t\tStep\t\t<0.1,2>\n"
            "\t\tPRT\t\t<0.1,1>\n"
            "\t\tPopSize\t\t<10,50>\n"
            "\t\tD\t\t<depend on problem>\n"
            "\t\tMigrations\t< depend on problem,  say 200-1000>\n"
            "\t\tMinDiv\t<0.1, depend on problem >\n"
            "\t\tLo\t\t(lower bound)\tfirst parameter\n"
            "\t\tHi\t\t(upper bound)\n"
            "\t\tType of parameter (0-real, 1-integer)\n"
            "\t\tLo\t\t(lower bound)\tsecond parameter\n"
            "\t\tHi\t\t(upper bound)\n"
            "\t\tType of parameter (0-real, 1-integer)\n"
            "\t\tEtc�(number of parameters from 1 to D)\n\n"
            "\tStructure of DEIN.TXT file is following (with recommended values):\n"
            "\t\tNP\t\t<10,10*NP>\n"
            "\t\tD\t\t<depend on problem>\n"
            "\t\tGenerations\t\t<1,depend on problem>\n"
            "\t\tF\t\t<0,2>\n"
            "\t\tCR\t\t<0,1>\n"
            "\t\tLo\t\t(lower bound)\tfirst parameter\n"
            "\t\tHi\t\t(upper bound)\n"
            "\t\tType of parameter (0-real, 1-integer)\n"
            "\t\tLo\t\t(lower bound)\tsecond parameter\n"
            "\t\tHi\t\t(upper bound)\n"
            "\t\tType of parameter (0-real, 1-integer)\n"
            "\t\tEtc�(number of parameters from 1 to D)\n\n"
            "************** End of Appendix ***************");
    fclose(in);

    //send history of evolution into file *.hst
    if ((in = fopen(HstFile, "w")) == NULL)
    {
        printf("Cannot open output file for history of evolution %s !!!\n",HstFile);
        goto END;
    }

    for(i=0;i<l;i++)
    {
        fprintf(in,"%d %lg\n",i+1,History[i]);
    }
    fclose(in);

END:;
}

void ShowInfo2(const char *ret)
{
    //on screen
    //clrscr();
    //return the best individual
    printf("Calculated by %s\n\n",ret);
    printf("Evolution conditions\nNP = %d,\tD = %d,\nF = %lg,\tCR = %lg,\n\n",NP, D, F,CR);

    //cost value of the best individual
    ckl1=getPopulation(PD-1,k);
    printf("\nCost value of the best individual  = %lg\n", getPopulation(PD-1,k));
    ckl1=getPopulation(PD-1,kk);
    printf("Cost value of the worst individual = %lg\n", ckl1);
    printf("\nGenerations = %d\n", l);
    printf("Number of function evaluations = %d\n\n>>>> PLEASE SEE FILE SOMAOUT.TXT or DEOUT.TXT <<<<\n\n", neval);

    //to file
    if ((in = fopen(OutFile, "w")) == NULL)
    {
        printf("Cannot open output file %s !!!\n",OutFile);
        goto END;
    }
    //return the best individual
    fprintf(in,"Calculated by %s\n\n",ret);
    fprintf(in,"Evolution conditions\nNP = %d,\tD = %d,\nF = %lg,\tCR = %lg,\n\n",NP, D, F,CR);
    fprintf(in,"\nBest individual\n");
    for(i=0;i<PD-1;i++)
    {
        //      ckl1=getPopulation(i,k);
        //		fprintf(in,"Parameter %d = %lg\n", i+1, ckl1);
        if(getSpecimen(2,i)==1)
        {
            ckl1=getIntPopulation(i,k);
            fprintf(in,"Parameter %d = %lg\n", i+1, ckl1);
        }
        else
        {
            ckl1=getPopulation(i,k);
            fprintf(in,"Parameter %d = %lg\n", i+1, ckl1);
        }
    }

    //cost value of the best individual
    ckl1=getPopulation(PD-1,k);
    fprintf(in,"Best individual cost value = %lg\n", ckl1);
    fprintf(in,"Generations = %d\n", l);
    fprintf(in,"Number of function evaluations = %d\n\n", neval);
    fprintf(in,
            "****************** Appendix ******************\n"
            "\nPLEASE DO NOT CHANGE TEXT IN ANY SOURCE CODE\n"
            "Updated by creator SOMA - Ivan Zelinka, 20.4.2001\n\n"
            "Source information:\n"
            "\tIf you are interested in more detail in SOMA or differential evolution\n"
            "\tthen please see:\n"
            "\thttp://www.ft.utb.cz/people/zelinka/soma.htm\n"
            "\tor contact :\n"
            "\t\tIvan Zelinka - zelinka@ft.utb.cz\n"
            "\t\tInstitute of Information Technologies, Faculty of Technology,\n"
            "\t\tThomas Bata University, Mostni 5139, 760 01 Zlin, Czech Republic\n\n"
            "\t\tJouni Lampinen - lampine@lut.fi\n"
            "\t\tLappeenranta University of Technology, Department of Information Technology,\n"
            "\t\tLaboratory of Information Processing, P.O.Box 20, FIN-53851 Lappeenranta, Finland\n\n"
            "How to handle this software:\n"
            "\tStructure of SOMAIN.TXT file is following (with recommended values):\n"
            "\t\tPathLength\t\t<0.1,5 >\n"
            "\t\tStep\t\t<0.1,2>\n"
            "\t\tPRT\t\t<0.1,1>\n"
            "\t\tPopSize\t\t<10,50>\n"
            "\t\tD\t\t<depend on problem>\n"
            "\t\tMigrations\t< depend on problem,  say 200-1000>\n"
            "\t\tMinDiv\t<0.1, depend on problem >\n"
            "\t\tLo\t\t(lower bound)\tfirst parameter\n"
            "\t\tHi\t\t(upper bound)\n"
            "\t\tType of parameter (0-real, 1-integer)\n"
            "\t\tLo\t\t(lower bound)\tsecond parameter\n"
            "\t\tHi\t\t(upper bound)\n"
            "\t\tType of parameter (0-real, 1-integer)\n"
            "\t\tEtc�(number of parameters from 1 to D)\n\n"
            "\tStructure of DEIN.TXT file is following (with recommended values):\n"
            "\t\tNP\t\t<10,10*NP>\n"
            "\t\tD\t\t<depend on problem>\n"
            "\t\tGenerations\t\t<1,depend on problem>\n"
            "\t\tF\t\t<0,2>\n"
            "\t\tCR\t\t<0,1>\n"
            "\t\tLo\t\t(lower bound)\tfirst parameter\n"
            "\t\tHi\t\t(upper bound)\n"
            "\t\tType of parameter (0-real, 1-integer)\n"
            "\t\tLo\t\t(lower bound)\tsecond parameter\n"
            "\t\tHi\t\t(upper bound)\n"
            "\t\tType of parameter (0-real, 1-integer)\n"
            "\t\tEtc�(number of parameters from 1 to D)\n\n"
            "************** End of Appendix ***************");
    fclose(in);

    //send history of evolution into file *.hst
    if ((in = fopen(HstFile, "w")) == NULL)
    {
        printf("Cannot open output file for history of evolution %s !!!\n",HstFile);
        goto END;
    }

    for(i=0;i<Generations;i++)
    {
        fprintf(in,"%d %lg\n",i+1,History[i]);
    }
    fclose(in);

END:;
}

double calc_sstot (Array2D *array, int column)
{
    int ind; 
    double mean=0; 
    double sstot=0;

    for (ind=0; ind<array->nrows; ind++)
        mean+=array->vals[ind][column];  //Y is column 0
    mean/=(double) array->nrows;
    for (ind=0; ind<array->nrows; ind++)
        sstot+=pow (array->vals[ind][column]-mean, 2);

    return sstot;
}

void CostValue(int Individual, Array2D *soma_array, MuExpr *soma_muexpr, double sstot)
{
    double	CostValue=0.0;
	int ParmOfInd;	//parameter of individual
    int i, j;
	double val;
	double zero=0;

    for (i=0; i<PD-1; i++) {
        if (getSpecimen (2,0)==1)
            CostValue=0; //never happens
        else
            soma_muexpr->pars[i]=getPopulation (i, Individual);
    }

    for (i=0; i<soma_array->nrows; i++) {
        for (j=1; j<soma_array->ncolumns; j++) 
            soma_muexpr->vars[j]=soma_array->vals[i][j];

        val=muexpr_eval (soma_muexpr);
        if (!(val==val)) 
			val=1.0/zero;   //prevent NaNs 
        CostValue+=pow(soma_array->vals[i][0]-val, 2);
    }

    setPopulation(PD-1,Individual,CostValue);
}

void CostValue2(int Individual, FFparam *ffp, Array2D *soma_array, FitnessFunction fitfunc, double sstot)
{
    double CostValue;
    int ParmOfInd;	//parameter of individual
    int i, j;

	CostValue=0.0;
    //PD-1 is = number of parameters
    for (i=0; i<PD-1; i++) {
        if (getSpecimen (2,0)==1)
            CostValue=0; //never happens
        else
            ffp[i].value=getPopulation (i, Individual);
    }

    for (i=0; i<soma_array->nrows; i++) {
        fitfunc (PD-1, ffp, soma_array->ncolumns-2, soma_array->vals[i]);
        CostValue+=pow(soma_array->vals[i][1]-soma_array->vals[i][0], 2);    
    }

    setPopulation(PD-1,Individual,CostValue);
}

