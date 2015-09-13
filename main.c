//Demo 

#include <math.h>
#include <stdio.h>
#include "lib.h"

int test (const char* file, const char *expr, int nparams);
int test2 (const char* file, FitnessFunction ffunc, int nparams);
void funcmcurve (int params, FFparam *ffp, int vars, double *array);
void funcsaw (int params, FFparam *ffp, int vars, double *array);
void funcwaterfall (int params, FFparam *ffp, int vars, double *array);

int main (int argc, char** argv)
{
    //3 data sets given, fitting calculated with 2 function variants

    printf ("Mountain Curve SOMAascii\n");
    test ("mcurve.csv", "(m1/m2)*exp(-0.5*((x-m3)^2/m2))", 3);
    printf ("Mountain Curve SOMAexternal\n");
    test2 ("mcurve.csv", funcmcurve, 3);
    
    printf ("Sawtooth SOMAascii\n");
    test ("sawtooth.csv", "m1*(1+4*m2^2*(x0/m3)^2)/(((1-(x0/m3)^2)^2)+4*(m2^2)*(x0/m3)^2)", 3);
    printf ("Sawtooth SOMAexternal\n");
    test2 ("sawtooth.csv", funcsaw, 3);

    printf ("Waterfall SOMAascii\n");
    test ("waterfall.csv", "m1*sin(x1^m2)*exp(cos(x2/m3))", 3);
    printf ("Waterfall SOMAexternal\n");
    test2 ("waterfall.csv", funcwaterfall, 3);

    return 0;
}

//helper function for demo - calls SOMAexternal
int test2 (const char* file, FitnessFunction ffunc, int nparams) 
{
    SOMAsetup sset={3,  //PathLength recommended value
        0.11,           //Step recommended value   
        0.1,            //Perturbation recommended value
        15,             //Size of population
        200,            //200 migrations
        -1,             //Stop condition MinDiv: -1 means MinDiv stop condition is never met
        1};             //R^2 threshold 

    //3 parameters: name, defaul value (unused), min, max
    Parameter par[3]={{"m1", 0, -1000, 1000}, {"m2", 0, -1000, 1000}, {"m3", 0, -1000, 1000}};
    int i;

    //output structure
    SOMAreturn ret;
    //Fitting function. Inputs: SOMA setup, external function, number of parameters,
    //parameter list, CSV file and output structure.
    //Memory inside output structure (history) is allocated by SOMAexternal
    SOMAexternal (&sset, ffunc, nparams, par, file, &ret);

    printf ("\nElapsed time: %f s\n", ret.time);
    printf ("R^2: %f\n", ret.history[ret.solution].Rsquared);
    for (i=0; i<nparams; i++) 
        printf ("Parameter %s = %e\n", par[i].name, ret.history[ret.solution].params[i]);
    printf ("----------------------------------\n");

    //Free memory allocated in output structure
    SOMAreturn_free (&ret);
    return OK;

}

//helper function for demo - calls SOMAascii
int test (const char* file, const char *expr, int nparams) 
{
    SOMAsetup sset={3, 0.11, 0.1, 15, 200, -1, 1};
    Parameter par[10]={{"m1", 0, -1000, 1000}, {"m2", 0, -1000, 1000}, {"m3", 0, -1000, 1000}};
    int i;

    SOMAreturn ret;
    //Fitting function. Inputs: SOMA setup, ascii equation, number of parameters,
    //parameter list, CSV file name and output structure.
    //Memory inside output structure (history) is allocated by SOMAexternal
    SOMAascii (&sset, expr, nparams, par, file, &ret);

    printf ("\nElapsed time: %f s\n", ret.time);
    printf ("R^2:%f \n", ret.history[ret.solution].Rsquared);
    for (i=0; i<nparams; i++) 
        printf ("Parameter %s = %e\n", par[i].name, ret.history[ret.solution].params[i]);
    printf ("----------------------------------\n");

    SOMAreturn_free (&ret);
    return OK;
}

//SOMAexternal fitness function for mountain curve data set
void funcmcurve (int params, FFparam *ffp, int vars, double *array)
{
    double m1=ffp[0].value;
    double m2=ffp[1].value;
    double m3=ffp[2].value;
    double x1=array[2];

    array[0]=(m1/m2)*exp(-0.5*((x1-m3)*(x1-m3))/m2);

}

//SOMAexternal fitness function for sawtooth data set
void funcsaw (int params, FFparam *ffp, int vars, double *array)
{
    double a1=ffp[0].value;
    double a2=ffp[1].value;
    double a3=ffp[2].value;
    double x0=array[2];

    array[0]=a1*(1+4*a2*a2*(x0/a3)*(x0/a3))/(pow((1-(x0/a3)*(x0/a3)), 2)+4*(a2*a2)*(x0/a3)*(x0/a3));
}

//SOMAexternal fitness function for waterfall data set
void funcwaterfall (int params, FFparam *ffp, int vars, double *array)
{
	double zero=0;

    double m1=ffp[0].value;
    double m2=ffp[1].value;
    double m3=ffp[2].value;
    double x1=array[2];
    double x2=array[3];

    array[0]=m1*sin(pow(x1,m2))*exp(cos(x2/m3));
    
   if (!(array[0]==array[0]))
        array[0]=1.0/zero;    //just to prevent nans 
}


