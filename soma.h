//SOMA algorithm 

#ifndef __SOMA_H
#define __SOMA_H

#include <stdio.h>
#include <stdlib.h>
#include "lib.h"

int SOMA2 (SOMAsetup*, int, Parameter*, SOMAreturn*, Array2D*, FitnessFunction);
int SOMA (SOMAsetup*, int, Parameter*, SOMAreturn*, Array2D*, MuExpr*);

int createPopulation(int pd, int pnp);
int destroyPopulation(void);
double getPopulation(int pd, int pnp);
double getIntPopulation(int pd, int pnp);
void setPopulation(int pd, int pnp, double pop);

int createTMPPopulation(void);
int destroyTMPPopulation(void);
double getTMPPopulation(int pd, int pnp);
void setTMPPopulation(int pd, int pnp, double pop);

int createPRTV(int ml);
int destroyPRTV(void);

int createHistory(int ml);
int destroyHistory(void);

int createSpecimen(int pd);
int destroySpecimen(void);
double getSpecimen(int dm, int pd);
void setSpecimen(int dm, int pd, double pop);

void WorstIndividual(void);
void BestIndividual(void);
void ShowInfo(const char *ret);
void ShowInfo2(const char *ret);

#endif 
