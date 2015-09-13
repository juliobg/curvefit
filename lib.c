//Libray implementation

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "lib.h"
#include "csv.h"
#include "muParserDLL.h"
#include "soma.h"


static int is_expr_var (MuExpr *mexpr, const char *name);

void muexpr_init (MuExpr *me, const char *exprstr)
{
    me->nvars=0;
    me->npars=0;
    me->hparser=mupCreate(muBASETYPE_FLOAT);
    mupSetExpr (me->hparser, exprstr);
}

void muexpr_free (MuExpr *me)
{
    mupRelease (me->hparser);
}

double muexpr_eval (MuExpr *me)
{
    return mupEval (me->hparser);
}

void array_init (Array2D *parray) 
{
    parray->vals=NULL;
    parray->nrows=1;
    parray->ncolumns=1;
    parray->vals=(double**) malloc (sizeof(double*));
    parray->vals[0]=(double*) malloc (sizeof(double));
    parray->currentrow=0;
    parray->currentcolumn=0;
}

void array_delete (Array2D *parray)
{
    int i;

    for (i=0; i<parray->nrows; i++)
        free (parray->vals[i]);

    free (parray->vals);
}

void array_append_row (Array2D *parray)
{
    parray->nrows++;
    parray->vals=(double**) realloc (parray->vals, parray->nrows*sizeof (double*));
    parray->vals[parray->nrows-1]=(double*) malloc (parray->ncolumns*sizeof(double));
    return;
}

void array_append_column (Array2D *parray)
{
    int i;
    parray->ncolumns++;

    for (i=0; i<parray->nrows; i++) 
        parray->vals[i]=(double*) realloc (parray->vals[i], parray->ncolumns*sizeof(double));

    return;
}

void array_print (Array2D *parray)
{
    int i, j;

    for (i=0; i<parray->nrows; i++) {
        for (j=0; j<parray->ncolumns; j++)
            printf ("%e, ", parray->vals[i][j]);
        printf ("\n");
    }
}

void SOMAreturn_init (SOMAreturn *sr, int migrations, int nparams) 
{
	int i;

    sr->history=(Leader*) malloc ((migrations+1)*sizeof(Leader)); //numbers of migratios + initial population

    for (i=0; i<(migrations+1); i++) {
        sr->history[i].params=(double*) malloc (nparams*sizeof(double));
    }

    sr->solution=0;
    sr->nleaders=migrations+1;
}

void SOMAreturn_free (SOMAreturn *sr)
{
    int i;
    for (i=0; i<sr->nleaders; i++)
        free (sr->history[i].params);

    free (sr->history);
}

void csvcallback1 (void *pfield, size_t size, void *parray) 
{
    Array2D *pa=(Array2D*) parray;
    char *chptr;

    if (pa->currentcolumn==pa->ncolumns)
        array_append_column (pa);

    if (pa->currentrow==pa->nrows)
        array_append_row (pa);

    pa->vals[pa->currentrow][pa->currentcolumn]=strtod ((char*)pfield, &chptr);

    pa->currentcolumn++;
    return;
}

void csvcallback2 (int c, void *parray)
{
    Array2D *pa=(Array2D*) parray;

    pa->currentrow++;
    pa->currentcolumn=0;

    return;
}

//currentcolumn=1: blank column for external fitness function
void csvcallback2e (int c, void *parray)
{
    Array2D *pa=(Array2D*) parray;

    pa->currentrow++;
    pa->currentcolumn=1;

    return;
}

int SOMAascii (SOMAsetup *ssetup, const char *expr, int nparams, Parameter *params, const char *file, SOMAreturn* sr) {
    Array2D array;
	int i;  
    char *buf;
    long len;
    struct csv_parser csvp;
    clock_t begin, end;
	MuExpr me;

	array_init(&array);

    //Parse CSV file
    if (csv_init (&csvp, CSV_APPEND_NULL)!=0)
        return ERR;

    if ((buf=read_file_into_buffer (file, &len))==NULL)
        return ERR_FILE;
    csv_parse (&csvp, buf, len, csvcallback1, csvcallback2, (void*) &array);
    csv_fini (&csvp, csvcallback1, csvcallback2, (void*) &array);
    csv_free (&csvp);

    //array_print (&array); //debug

    muexpr_init (&me, expr);

    for (i=0; i<nparams; i++) 
        if (defpar (&me, params+i, i)!=OK) {
            return ERR_PARAMS;
            array_delete (&array);
            muexpr_free (&me);
        }
    findvars (&me, VAR_NAME_PREFIX);

    //CVS file and expression don't match
    if (me.nvars!=array.ncolumns-1) {
        array_delete (&array);
        muexpr_free (&me);
        return ERR_COLUMNS;
    }

    muexpr_eval (&me);
    if (mupError (me.hparser)) {
        array_delete (&array);
        muexpr_free (&me);
        return ERR_EXPR;
    }

    //Initialize output struct
    SOMAreturn_init (sr, ssetup->Migrations, nparams);
    begin=clock();
    //SOMA call
    SOMA (ssetup, nparams, params, sr, &array, &me);
    end=clock();
    sr->time=(double) (end-begin)/CLOCKS_PER_SEC;
    array_delete (&array);
    muexpr_free (&me);

    return OK;
}

int SOMAexternal (SOMAsetup *ssetup, FitnessFunction ffunc, int nparams, Parameter *params, const char *file, SOMAreturn* sr) {
    Array2D array;
    char *buf;
    long len;
    struct csv_parser csvp;
    clock_t begin, end;
	
	array_init (&array);
	array_append_column(&array); //blank column for external fitness function
    array.currentcolumn++;

    //Parse CSV file
    if (csv_init (&csvp, CSV_APPEND_NULL)!=0)
        return ERR;

    if ((buf=read_file_into_buffer (file, &len))==NULL)
        return ERR_FILE;
    csv_parse (&csvp, buf, len, csvcallback1, csvcallback2e, (void*) &array);
    csv_fini (&csvp, csvcallback1, csvcallback2e, (void*) &array);
    csv_free (&csvp);

    //array_print (&array); //debug

    //soma_array=&array;
    //Initialize output struct
    SOMAreturn_init (sr, ssetup->Migrations, nparams);
    begin=clock();
    //SOMA call
    SOMA2 (ssetup, nparams, params, sr, &array, ffunc);
    end=clock();
    sr->time=(double) (end-begin)/CLOCKS_PER_SEC;
    array_delete (&array);

    return OK;
}


int defpar (MuExpr *mexpr, const Parameter *param, int i)
{
    if (i>=MAX_PARS)
        return ERR;
    mupDefineVar (mexpr->hparser, param->name, mexpr->pars+i);
    if (is_expr_var (mexpr, param->name)==ERR)
        return ERR;

    mexpr->npars++;
    return OK;
}

int findvars (MuExpr *mexpr, const char *name)
{
    char varname[MAX_VAR_NAME_LEN];
    int i, j=0;

    strcpy (varname, name);
    for (i=0; i<MAX_VARS-1; i++) {
        if (is_expr_var (mexpr, varname)==ERR) {
            if (mexpr->nvars!=0 && i!=1) //Don't break if next variable is x1 or no variables found yet 
                break;
            else {
				sprintf (varname, "%s%d", name, i);
                //MS Visual C++ does not support snprintf
                //snprintf (varname, MAX_VAR_NAME_LEN, "%s%d", name, i);
                continue;
            }
        }
        mupDefineVar (mexpr->hparser, varname, mexpr->vars+j+1); //First variable is mexpr->vars[1]
        mexpr->nvars++;
        j++;
		sprintf (varname, "%s%d", name, i);
        //MS Visual C++ does not support snprintf
        //snprintf (varname, MAX_VAR_NAME_LEN, "%s%d", name, i);
    }

    return OK;
}

int is_expr_var (MuExpr *mexpr, const char *name)
{
    unsigned int i, j;
    const char *charptr;
    muFloat_t *floatptr;

    j=mupGetExprVarNum (mexpr->hparser);

    for (i=0; i<j; i++) {
        mupGetExprVar (mexpr->hparser, i, &charptr, &floatptr);
        if (!strcmp (charptr, name))
            return OK;
    }

    return ERR;
}

char* read_file_into_buffer (const char *filename, long* len)
{
	long fsize;
    FILE *f;
	char *buffer;
	f = fopen(filename, "r");
    if (f==NULL)
        return NULL;

    fseek (f, 0, SEEK_END);
    fsize = ftell(f);
    fseek (f, 0, SEEK_SET);
    *len=fsize;

    buffer = (char*) malloc (fsize+1);
    fread (buffer, fsize, 1, f);
    fclose (f);

    buffer[fsize]='\0';

    return buffer;
}
