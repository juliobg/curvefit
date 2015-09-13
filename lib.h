//Library header

#ifndef SOMA_LIB_H
#define SOMA_LIB_H

#include "muParserDLL.h"

#ifdef __cplusplus
extern "C" {
#endif

//Return codes Functions that return int
#define OK 0        //No error
#define ERR -1      //General error
#define ERR_FILE -2 //Error opening or reading file
#define ERR_PARAMS -3 //Error: too many parameters or parameter not found
#define ERR_COLUMNS -4 //Error: not enough columns in CSV file (deppending on equation string);
#define ERR_EXPR -5 //Error: Invalid muparser expression or undefined variables/parameters

#define MAX_PAR_NAME_LEN 4096   //Max par name length
#define MAX_VARS 4096           //Max number of variables (x1, x2, x3...)
#define MAX_PARS 4096           //Max number of parameters
#define MAX_VAR_NAME_LEN 4096   //Max variable name length
#define VAR_NAME_PREFIX  "x"    //Prepended before variable's name
                                //SOMAascii will look variables named
                                //VAR_NAME_PREFIX, VAR_NAME_PREFIX0,
                                //VAR_NAME_PREFIX1 ...

//Parameter struct for SOMAascii and SOMAexternal
typedef struct parameter_ {
    char name[MAX_PAR_NAME_LEN];    //max parameter name length
    double initvalue;
    double min;                     //parameter bounds
    double max;                     //parameter bounds
} Parameter;

//SOMA setup structure
typedef struct somasetup_ {
    double PathLength;
    double Step;
    double PRT;
    int PopSize;
    int Migrations;
    double MinDiv;
    double PRV;
} SOMAsetup;

//Muparser helper structure
typedef struct muexpr_ {
    double vars[MAX_VARS];      //Allocated storage for expression variables (x1, x2, x3..)
    double pars[MAX_PARS];      //Allocated storage for expression parameters (m1, m2)
    muParserHandle_t hparser;   //muparser handle
    int nvars;                  //number of variables
    int npars;                  //number of parameters (also variables for muparser)
} MuExpr;

//2D Array
typedef struct array2d_ {
    double **vals;      //2D array: vals[row][column]
    int nrows;          //number of rows
    int ncolumns;       //number of columns
    int currentrow;     //used in parsing
    int currentcolumn;  //used in parsing
} Array2D;

//Migration leader struct
typedef struct leader_ {    //Migration leader
    double *params;         //Parameters
    double Rsquared;        //R^2
} Leader;

//Output structure
typedef struct somareturn_ {
    Leader *history;        //Array of migration leader structs
    int nleaders;           //Number or leaders in history
    int solution;           //Index of solution
    double time;            //elapsed time
} SOMAreturn;

//Parameter pair for fitness function
typedef struct ffparam_ {
    char name [MAX_PAR_NAME_LEN];
    double value;
} FFparam;

//SOMAexternal computation function type
typedef void (*FitnessFunction) (int, FFparam*, int, double*); 

//Multivariable regression function (ascii)
int SOMAascii (SOMAsetup *ssetup, const char *expr, int nparams, Parameter *params, const char *file, SOMAreturn*);
//Multivariable regression function (external)
int SOMAexternal (SOMAsetup *ssetup, FitnessFunction, int nparams, Parameter *params, const char *file, SOMAreturn*);

//Init Array2D (necessary before using)
void array_init (Array2D *parray);

//Free Array2D memory
void array_delete (Array2D *parray);

//Append row
void array_append_row (Array2D *parray);

//Append column
void array_append_column (Array2D *parray);

//Used by CSV parser
void csvcallback1 (void *pfield, size_t size, void *parray);
void csvcallback2 (int c, void *parray); 

//Read file into memory
char* read_file_into_buffer (const char *filename, long* len);

//Prints Array2D (debugging)
void array_print (Array2D *parray);

//Initi MuExpr with ascii equation
void muexpr_init (MuExpr *me, const char *exprstr);

//Release muparser handle
void muexpr_free (MuExpr*);

//Evaluate ascii equation
double muexpr_eval (MuExpr *me);

//Used by SOMAascii
int defpar (MuExpr *mexpr, const Parameter *param, int i);

//Used by SOMAascii
int findvars (MuExpr *mexpr, const char *name);

//SOMAascii and SOMAexternal output structure initialization (called
//internally SOMAascii and SOMAexternal)
void SOMAreturn_init (SOMAreturn *sr, int migrations, int nparams);

//Free memory allocated in output structure
void SOMAreturn_free (SOMAreturn *sr);

#ifdef __cplusplus
}
#endif

#endif
