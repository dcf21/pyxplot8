// ppl_userspace.c
//
// The code in this file is part of PyXPlot
// <http://www.pyxplot.org.uk>
//
// Copyright (C) 2006-9 Dominic Ford <coders@pyxplot.org.uk>
//               2008-9 Ross Church
//
// $Id$
//
// PyXPlot is free software; you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the Free Software
// Foundation; either version 2 of the License, or (at your option) any later
// version.
//
// You should have received a copy of the GNU General Public License along with
// PyXPlot; if not, write to the Free Software Foundation, Inc., 51 Franklin
// Street, Fifth Floor, Boston, MA  02110-1301, USA

// ----------------------------------------------------------------------------

#define _PPL_USERSPACE 1

// Include <ppl_units.c> as a part of this source file to allow inline
// compilation of arithmetic functions.
#include "ppl_units.c"
#include "MathsTools/dcfmath.c"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <math.h>

#include <gsl/gsl_math.h>
#include <gsl/gsl_spline.h>

#include "StringTools/asciidouble.h"
#include "StringTools/str_constants.h"

#include "ListTools/lt_dict.h"

#include "ppl_calculus.h"
#include "ppl_constants.h"
#include "ppl_interpolation.h"
#include "ppl_userspace.h"

// -------------------------------------------------------------------
// Data structures used for storing the user's variables and functions
// -------------------------------------------------------------------

Dict *_ppl_UserSpace_Vars;
Dict *_ppl_UserSpace_Funcs;

// ppl_UserSpace_SetVarStr(): Called to define a new string variable within the user's variable space
void ppl_UserSpace_SetVarStr(char *name, char *inval, int modified)
 {
  value v;
  ppl_units_zero(&v);
  v.modified = modified;
  v.string   = inval;
  DictAppendValue(_ppl_UserSpace_Vars , name , v);
  return;
 }

// ppl_UserSpace_SetVarNumeric(): Called to define a new numeric variable within the user's variable space
void ppl_UserSpace_SetVarNumeric(char *name, value *inval, int modified)
 {
  inval->modified = modified;
  DictAppendValue(_ppl_UserSpace_Vars , name , *inval);
  return;
 }

// ppl_UserSpace_UnsetVar(): Called to unset a variable definition
void ppl_UserSpace_UnsetVar(char *name)
 {
  DictRemoveKey(_ppl_UserSpace_Vars , name);
  return;
 }

// ppl_UserSpace_SetFunc(): Called to define a new algebraic function definition.
void ppl_UserSpace_SetFunc(char *definition, int modified, int *status, char *errtext)
 {
  int i=0, j, k, supersede, name_i=0, Nargs=0, args_i=0, args_j=0, lcount=0;
  int Nsupersede, Noverlap, Nmiss, LastOverlapType, LastOverlapK, NeedToStoreNan;
  double tempdbl;
  char name[LSTR_LENGTH] , args[LSTR_LENGTH];
  value min[ALGEBRA_MAXITEMS], max[ALGEBRA_MAXITEMS];
  unsigned char MinActive[ALGEBRA_MAXITEMS], MaxActive[ALGEBRA_MAXITEMS];
  FunctionDescriptor *OldFuncPtr, *OldFuncIter, **OldFuncPrev, *NewFuncPtr, *temp;

  for (j=0; j<ALGEBRA_MAXITEMS; j++) MinActive[j]=0; // By default, function definition has no limits to the range over which it is applicable
  for (j=0; j<ALGEBRA_MAXITEMS; j++) MaxActive[j]=0;

  while ((definition[i]>'\0')&&(definition[i]<=' ')) i++; // First of all, read the name of the new function into name[]
  while ((definition[i]>'\0')&&(((name_i==0)&&isalpha(definition[i]))||((name_i!=0)&&(isalnum(definition[i])||(definition[i]=='_'))))) name[name_i++] = definition[i++];
  name[name_i++] = '\0';
  if (name_i==0) { *status=1 ; strcpy(errtext, "Could not read function name"); return; }
  while ((definition[i]>'\0')&&(definition[i]<=' ')) i++;
  if (definition[i]!='(') { *status=1 ; strcpy(errtext, "Function name should be followed by ()"); return; }
  i++;
  while ((definition[i]>'\0')&&(definition[i]<=' ')) i++;
  while (definition[i]!=')') // Loop over a comma-separated list of argument names within ()
   {
    args_j=0; // Read the names into args[], separated by \0 characters. The number of arguments read is Nargs.
    while ((definition[i]>'\0')&&(((args_j==0)&&isalpha(definition[i]))||((args_j!=0)&&(isalnum(definition[i])||(definition[i]=='_'))))) args[args_i + (args_j++)] = definition[i++];
    args[args_i + (args_j++)] = '\0';
    if (args_j==0) { *status=1 ; strcpy(errtext, "Unexpected characters in list of arguments"); return; }
    Nargs++; args_i += args_j;
    if (Nargs>ALGEBRA_MAXITEMS) { *status=1 ; sprintf(errtext, "Functions are not permitted to take more than %d arguments", ALGEBRA_MAXITEMS); return; }
    while ((definition[i]>'\0')&&(definition[i]<=' ')) i++;
    if (definition[i]!=',') break;
    i++;
    while ((definition[i]>'\0')&&(definition[i]<=' ')) i++;
   }
  if (definition[i]!=')') { *status=1 ; strcpy(errtext, "Mismatched bracket in list of function arguments"); return; }
  i++;
  while ((definition[i]>'\0')&&(definition[i]<=' ')) i++;
  while (definition[i]=='[') // Loop over a set of ranges specified with the syntax, for example [1:2]
   {
    if (lcount==Nargs) { *status=1 ; strcpy(errtext, "Function definition contains more range specifications than arguments to apply them to"); return; }
    i++;
    while ((definition[i]>'\0')&&(definition[i]<=' ')) i++;
    if       (definition[i]==':')                            {} // Is we have a :, the user has not specified a minimum bound
    else if ((definition[i]=='t') && (definition[i+1]=='o')) {}
    else if  (definition[i]=='*')                            { i++; }
    else
     {
      *status=-1; args_j=0;
      ppl_EvaluateAlgebra(definition+i, min+lcount, 0, &args_j, status, errtext, 0);
      if (*status >= 0) return;
      i+=args_j;
      if (min[lcount].FlagComplex) { *status=1; sprintf(errtext, "Where ranges are specified for function arguments, these must be real numbers. Ranges may not be imposed upon complex arguments to functions."); return; }
      MinActive[lcount]=1;
     }
    while ((definition[i]>'\0')&&(definition[i]<=' ')) i++;

    if       (definition[i]==':')                            i++;
    else if ((definition[i]=='t') && (definition[i+1]=='o')) i+=2;
    else       { *status=1 ; strcpy(errtext, "Unexpected characters when searching for : in range specification"); return; }
    while ((definition[i]>'\0')&&(definition[i]<=' ')) i++;

    if      (definition[i]==']') {} // If we have a ], the user has not specified a maximum bound
    else if (definition[i]=='*') { i++; }
    else
     {
      *status=-1; args_j=0;
      ppl_EvaluateAlgebra(definition+i, max+lcount, 0, &args_j, status, errtext, 0);
      if (*status >= 0) return;
      i+=args_j;
      if (max[lcount].FlagComplex) { *status=1; sprintf(errtext, "Where ranges are specified for function arguments, these must be real numbers. Ranges may not be imposed upon complex arguments to functions."); return; }
      if ((MinActive[lcount]) && (!ppl_units_DimEqual(min+lcount , max+lcount))) { *status=1; sprintf(errtext, "The range specified for argument number %d to this function has a minimum with dimensions of <%s>, and a maximum with dimensions of <%s>. This is not allowed: both must have the same dimensions.",lcount+1,ppl_units_GetUnitStr(min+lcount,NULL,NULL,0,0),ppl_units_GetUnitStr(max+lcount,NULL,NULL,1,0)); return; }
      MaxActive[lcount]=1;
     }
    while ((definition[i]>'\0')&&(definition[i]<=' ')) i++;

    if (definition[i]!=']') { *status=1 ; strcpy(errtext, "Unexpected characters when searching for ] in range specification"); return; }
    i++;

    // Swap min/max limits if they are given the wrong way around
    if ((MinActive[lcount]==1)&&(MaxActive[lcount]==1)&&(min[lcount].real>max[lcount].real))
     {
      tempdbl          = max[lcount].real;
      max[lcount].real = min[lcount].real;
      min[lcount].real = tempdbl; 
     }

    lcount++;
    while ((definition[i]>'\0')&&(definition[i]<=' ')) i++;
   }

  if (definition[i]!='=') { *status=1 ; strcpy(errtext, "Unexpected characters when searching for = in function definition"); return; }
  i++;
  while ((definition[i]>'\0')&&(definition[i]<=' ')) i++; // definition+i now points to the algebraic definition of the function

  DictLookup(_ppl_UserSpace_Funcs, name, NULL, (void *)&OldFuncPtr); // Check whether we are going to overwrite an existing function
  OldFuncIter =  OldFuncPtr;
  OldFuncPrev = &OldFuncPtr;
  NeedToStoreNan = 0;
  while (OldFuncIter != NULL)
   {
    if ((OldFuncIter->FunctionType==PPL_USERSPACE_SYSTEM)||(OldFuncIter->FunctionType==PPL_USERSPACE_STRFUNC)||(OldFuncIter->FunctionType==PPL_USERSPACE_UNIT))
        { *status=1 ; strcpy(errtext, "Attempt to redefine a core system function"); return; }
    supersede = 0;
    if ((supersede==0) && (OldFuncIter->FunctionType!=PPL_USERSPACE_USERDEF)) supersede=1; // Do not splice user-defined functions with splines

    if ((supersede==0) && (OldFuncIter->NumberArguments != Nargs)) supersede=1; // If old function has different number of arguments, we cannot splice with it; supersede it

    // If old function has dimensionally incompatible limits with us, we cannot splice with it; supersede it
    if (supersede==0) for (k=0; k<Nargs; k++) if ( ((MinActive[k]==1)&&(OldFuncIter->MinActive[k]==1)&&(!ppl_units_DimEqual(min+k , OldFuncIter->min+k))) ||
                                                   ((MaxActive[k]==1)&&(OldFuncIter->MaxActive[k]==1)&&(!ppl_units_DimEqual(max+k , OldFuncIter->max+k)))    ) supersede=1;

    // If old function has ranges which extend outside our range, we should not supersede it
    Nsupersede = Noverlap = Nmiss = LastOverlapType = LastOverlapK = 0;
    if (supersede==0) for (k=0; k<Nargs; k++) // Looping over all dimensions, count how new ranges miss/overlap/supersede old ranges
     {
      if      ( ((MinActive[k]==0)||((OldFuncIter->MinActive[k]==1)&&(OldFuncIter->min[k].real>=min[k].real))) &&
                ((MaxActive[k]==0)||((OldFuncIter->MaxActive[k]==1)&&(OldFuncIter->max[k].real<=max[k].real)))    ) Nsupersede++; // New min/max range completely encompasses old
      else if ( ((MinActive[k]==1)&& (OldFuncIter->MaxActive[k]==1)&&(OldFuncIter->max[k].real<=min[k].real) ) ||
                ((MaxActive[k]==1)&& (OldFuncIter->MinActive[k]==1)&&(OldFuncIter->min[k].real>=max[k].real) )    ) Nmiss++; // New min/max range completely outside old
      else if (  (MinActive[k]==1)&&((OldFuncIter->MinActive[k]==0)||(OldFuncIter->min[k].real< min[k].real)) &&
                 (MaxActive[k]==1)&&((OldFuncIter->MaxActive[k]==0)||(OldFuncIter->max[k].real> max[k].real))     )  { Noverlap++; LastOverlapType = 2; LastOverlapK = k; } // New range in middle of old
      else if (  (MinActive[k]==1)&&(((OldFuncIter->MaxActive[k]==1)&&(OldFuncIter->max[k].real> min[k].real)  &&
                                     ((MaxActive[k]             ==0)||(OldFuncIter->max[k].real<=max[k].real)))||
                                     ((OldFuncIter->MaxActive[k]==0)&&(MaxActive[k]            == 0           ))    )) { Noverlap++; LastOverlapType = 3; LastOverlapK = k; } // New range goes off top of old
      else if (  (MaxActive[k]==1)&&(((OldFuncIter->MinActive[k]==1)&&(OldFuncIter->min[k].real< max[k].real)  &&
                                     ((MinActive[k]             ==0)||(OldFuncIter->min[k].real>=min[k].real)))||
                                     ((OldFuncIter->MinActive[k]==0)&&(MinActive[k]            == 0           ))    )) { Noverlap++; LastOverlapType = 1; LastOverlapK = k; } // New range goes off bottom of old
      else  ppl_fatal(__FILE__,__LINE__,"Could not work out how the ranges of two functions overlap");
     }

    if ((supersede > 0) || ((Nmiss==0) && (Noverlap==0))) // Remove entry from linked list; either old definition is incompatible, or we supersede its range on all axes
     {
      ppl_UserSpace_FuncDestroy(OldFuncIter);
      *OldFuncPrev = OldFuncIter->next;
      supersede = 1;
     }
    else if ((Nmiss==0) && (Noverlap==1)) // we should reduce the range of the function we overlap with
     {
      if      (LastOverlapType == 1) // Bring lower limit of old definition up above maximum for this new definition
       { OldFuncIter->min[LastOverlapK].real = max[LastOverlapK].real; OldFuncIter->MinActive[LastOverlapK] = 1; }
      else if (LastOverlapType == 3) // Bring upper limit of old definition down below minimum for this new definition
       { OldFuncIter->max[LastOverlapK].real = min[LastOverlapK].real; OldFuncIter->MaxActive[LastOverlapK] = 1; }
      else
       {
        ppl_UserSpace_FuncDuplicate(OldFuncIter , modified); // Old definition is cut in two by the new definition; duplicate it.
        OldFuncIter->max[LastOverlapK].real        = min[LastOverlapK].real;
        OldFuncIter->MaxActive[LastOverlapK]       = 1;
        OldFuncIter->next->min[LastOverlapK].real  = max[LastOverlapK].real;
        OldFuncIter->next->MinActive[LastOverlapK] = 1;
       }
     }
    else if ((strlen(definition+i)==0) && (Nmiss==0) && (Noverlap>1)) // We are trying to undefine the function, but overlap is complicated along > 1 axis. Best we can do is store the undefinition.
     {
      NeedToStoreNan = 1;
     }

    temp=OldFuncIter->next;
    if (supersede > 0) { free(OldFuncIter); } else { OldFuncPrev = &(OldFuncIter->next); }
    OldFuncIter=temp;
   }

  // If we have a blank algebraic expression for this function, do not add any new definition for this function, unless we have complicated overlap issues
  if ((strlen(definition+i)==0) && (NeedToStoreNan == 0))
   {
    if (OldFuncPtr == NULL) DictRemoveKey(_ppl_UserSpace_Funcs, name);
    else                    DictAppendPtr(_ppl_UserSpace_Funcs, name, (void *)OldFuncPtr, sizeof(FunctionDescriptor), 0, DATATYPE_VOID);
    return;
   }

  // Make a new function descriptor with the details of the new expression for this function
  if ((NewFuncPtr = (FunctionDescriptor *)lt_malloc_incontext(sizeof(FunctionDescriptor),0))==NULL) return;
       NewFuncPtr->FunctionType    = PPL_USERSPACE_USERDEF;
       NewFuncPtr->modified        = modified;
       NewFuncPtr->NumberArguments = Nargs;
  if ((NewFuncPtr->FunctionPtr     =          lt_malloc_incontext(strlen(definition+i)+1,0))==NULL) return; strcpy((char *)NewFuncPtr->FunctionPtr, definition+i );
  if ((NewFuncPtr->ArgList         = (char  *)lt_malloc_incontext(args_i                ,0))==NULL) return; memcpy(        NewFuncPtr->ArgList    , args        , args_i );
  if ((NewFuncPtr->min             = (value *)lt_malloc_incontext(Nargs * sizeof(value) ,0))==NULL) return; memcpy(        NewFuncPtr->min        , min         , Nargs*sizeof(value));
  if ((NewFuncPtr->max             = (value *)lt_malloc_incontext(Nargs * sizeof(value) ,0))==NULL) return; memcpy(        NewFuncPtr->max        , max         , Nargs*sizeof(value));
  if ((NewFuncPtr->MinActive       = (unsigned char *)lt_malloc_incontext(Nargs         ,0))==NULL) return; memcpy(        NewFuncPtr->MinActive  , MinActive   , Nargs);
  if ((NewFuncPtr->MaxActive       = (unsigned char *)lt_malloc_incontext(Nargs         ,0))==NULL) return; memcpy(        NewFuncPtr->MaxActive  , MaxActive   , Nargs);
       NewFuncPtr->next            = OldFuncPtr;
       NewFuncPtr->description     = NewFuncPtr->FunctionPtr;
       NewFuncPtr->LaTeX           = NULL;
  DictAppendPtr(_ppl_UserSpace_Funcs, name, (void *)NewFuncPtr, sizeof(FunctionDescriptor), 0, DATATYPE_VOID);
  return;
 }

// Free the data stored in a function descriptor
void ppl_UserSpace_FuncDestroy(FunctionDescriptor *in)
 {
  if (in->FunctionType == PPL_USERSPACE_USERDEF)
   {
    if (in->FunctionPtr!=NULL) free(in->FunctionPtr);
   }
  else if (in->FunctionType == PPL_USERSPACE_SPLINE)
   {
    if (in->FunctionPtr!=NULL) gsl_spline_free((gsl_spline *)in->FunctionPtr);
   }
  else
   {
    ppl_fatal(__FILE__,__LINE__,"Attempt to destroy a function descriptor which is neither a user-defined function, nor a spline.");
   }
  if (in->ArgList    !=NULL) free(in->ArgList);
  if (in->min        !=NULL) free(in->min);
  if (in->max        !=NULL) free(in->max);
  if (in->MinActive  !=NULL) free(in->MinActive);
  if (in->MaxActive  !=NULL) free(in->MaxActive);
  if (in->LaTeX      !=NULL) free(in->LaTeX);
  if ( (in->description!=NULL) && (in->description!=(char *)in->FunctionPtr) ) free(in->description);
  return;
 }

// Duplicate a user-defined function descriptor
void ppl_UserSpace_FuncDuplicate(FunctionDescriptor *in, int modified)
 {
  FunctionDescriptor *NewFuncPtr;
  int i,j;
  int Nargs;
  if (in->FunctionType != PPL_USERSPACE_USERDEF) ppl_fatal(__FILE__,__LINE__,"Attempt to duplicate a function descriptor which is not a user-defined function.");

  Nargs=in->NumberArguments;
  for (j=0,i=0;i<Nargs;i++) while (in->ArgList[j++]!='\0')

  if ((NewFuncPtr = (FunctionDescriptor *)lt_malloc_incontext(sizeof(FunctionDescriptor),0))==NULL) return;
       NewFuncPtr->FunctionType    = PPL_USERSPACE_USERDEF;
       NewFuncPtr->modified        = modified;
       NewFuncPtr->NumberArguments = in->NumberArguments;
  if ((NewFuncPtr->FunctionPtr     =          lt_malloc_incontext(strlen(in->FunctionPtr)+1,0))==NULL) return; strcpy((char *)NewFuncPtr->FunctionPtr, (char *)in->FunctionPtr );
  if ((NewFuncPtr->ArgList         = (char  *)lt_malloc_incontext(j                        ,0))==NULL) return; memcpy(        NewFuncPtr->ArgList    , in->ArgList     , j );
  if ((NewFuncPtr->min             = (value *)lt_malloc_incontext(Nargs * sizeof(value)    ,0))==NULL) return; memcpy(        NewFuncPtr->min        , in->min         , Nargs*sizeof(value));
  if ((NewFuncPtr->max             = (value *)lt_malloc_incontext(Nargs * sizeof(value)    ,0))==NULL) return; memcpy(        NewFuncPtr->max        , in->max         , Nargs*sizeof(value));
  if ((NewFuncPtr->MinActive       = (unsigned char *)lt_malloc_incontext(Nargs            ,0))==NULL) return; memcpy(        NewFuncPtr->MinActive  , in->MinActive   , Nargs);
  if ((NewFuncPtr->MaxActive       = (unsigned char *)lt_malloc_incontext(Nargs            ,0))==NULL) return; memcpy(        NewFuncPtr->MaxActive  , in->MaxActive   , Nargs);
       NewFuncPtr->next            = in->next;
       NewFuncPtr->description     = NewFuncPtr->FunctionPtr;
       NewFuncPtr->LaTeX           = NULL;
       in->next                    = NewFuncPtr;
       in->modified                = modified;
  return;
 }

// ---------------------------------------------------------------------------------
// ppl_GetQuotedString(): Extract a string from an input of the form "...." % (....)
//
// in: pointer to string from which quoted string is to be extracted
// out: string output is written here
// start: start scanning at in+start
// end: If NULL, does nothing. If *end>0, we must use all characters up to this point. If *end<0, last character of string is written here
// Local?Vars: Local variables which take precedence over global variables. 2 takes precedence over 1
// errpos: Set >=0 if an error is found at (in+errpos).
// errtext: Error messages are written here
// ---------------------------------------------------------------------------------

void ppl_GetQuotedString(char *in, char *out, int start, int *end, int *errpos, char *errtext, int RecursionDepth)
 {
  char QuoteType;
  char FormatString[LSTR_LENGTH]; // Used to store the string found between quotes, before applying the substitution operator
  char FormatArg   [LSTR_LENGTH]; // Used to store string arguments in the %(...) part of a quoted string
  char FormatToken [SSTR_LENGTH];
  int  pos      = start;
  int  outpos   = 0;
  value *VarData = NULL;
  int  pos2, CommaPositions[MAX_STR_FORMAT_ITEMS], NFormatItems;
  int  i,j,k,l,NArgs,ArgCount,RequiredArgs;
  char AllowedFormats[] = "cdieEfgGosSxX%"; // These tokens are allowed after a % format specifier

  FunctionDescriptor *FuncDefn = NULL;
  value ResultBuffer[ALGEBRA_MAXITEMS];       // A buffer of temporary numerical results

  int    arg1i, arg2i; // Integers for passing to sprintf
  value  arg1v, arg2v, argtemp; // Doubles (well, actually values with units) for passing to sprintf
  struct {double d,d2; int i,i2; char *s; unsigned int u,u2; value v; } argf; // We pass this lump of memory to sprintf

  if (RecursionDepth > MAX_RECURSION_DEPTH) { *errpos=start; strcpy(errtext,"Overflow Error: Maximum recursion depth exceeded."); return; }

  while ((in[pos]>'\0') && (in[pos]<=' ')) pos++; // Fast-forward over preceding spaces

  if ((in[pos]!='\'') && (in[pos]!='\"')) // If quoted string doesn't start with quote, it must be a string variable name
   {
    while (((isalnum(in[pos]))||(in[pos]=='_')) && ((end==NULL)||(*end<0)||(pos<*end))) FormatString[outpos++]=in[pos++]; // Fetch a word
    FormatString[outpos] = '\0';
    while ((in[pos]>'\0') && (in[pos]<=' ')) pos++; // Fast-forward over trailing spaces
    if (in[pos]=='(') // We have a function
     {
      DictLookup(_ppl_UserSpace_Funcs, FormatString, NULL, (void **)&FuncDefn); // Look up in database of function definitions
      if (FuncDefn == NULL) { *errpos = start; sprintf(errtext, "No such function, '%s'.", FormatString); return; }
      if (FuncDefn->FunctionType != PPL_USERSPACE_STRFUNC) { *errpos = start; sprintf(errtext, "This function returns a numeric result; a string function was expected."); return; }
      NArgs = FuncDefn->NumberArguments;
      pos++;
      for (k=0; k<NArgs; k++) // Now collect together numeric arguments
       {
        if (k+2 >= ALGEBRA_MAXITEMS) { *errpos = pos; strcpy(errtext,"Internal error: Temporary results buffer overflow."); return; }
        while ((in[pos]>'\0')&&(in[pos]<=' ')) pos++;
        if (in[pos]==')') { *errpos = pos; strcpy(errtext,"Syntax Error: Too few arguments supplied to function."); return; }
        j=-1;
        ppl_EvaluateAlgebra(in+pos, ResultBuffer+k+2, 0, &j, errpos, errtext, RecursionDepth+1);
        if (*errpos >= 0) { (*errpos) += pos; return; }
        pos+=j;
        while ((in[pos]>'\0')&&(in[pos]<=' ')) pos++;
        if (k < NArgs-1)
         {
          if (in[pos] != ',')
           {
            *errpos = pos;
            if (in[pos] ==')') strcpy(errtext,"Syntax Error: Too few arguments supplied to function.");
            else               strcpy(errtext,"Syntax Error: Unexpected trailing matter after argument to function.");
            return;
           } else { pos++; }
         }
       }
      if (NArgs==-1) // We have a function which takes one string argument
       {
        pos--; // pos should point to opening bracket
        StrBracketMatch(in+pos,NULL,NULL,&k,0);
        if (k<=0) { *errpos = pos; strcpy(errtext,"Syntax Error: Mismatched bracket."); return; }
        j=-1;
        ((void(*)(char*,int,value*,int*,char*))FuncDefn->FunctionPtr)(in+pos+1,k-1,ResultBuffer,&j,errtext);
        if (j>=0) { *errpos = pos+1+j; return; }
        pos += k;
       }
      if (in[pos] != ')') // Check that we have closing bracket
       {
        (*errpos) = pos;
        if (in[pos] ==',') strcpy(errtext,"Syntax Error: Too many arguments supplied to function.");
        else               strcpy(errtext,"Syntax Error: Unexpected trailing matter after final argument to function");
        return;
       } else { pos++; }
      if ((end!=NULL)&&(*end>0)&&(pos<*end)) { *errpos = pos; strcpy(errtext, "Syntax Error: Unexpected trailing matter after function call."); return; } // Have we used up as many characters as we were told we had to?
      j=0;
      if      (NArgs>=0) ResultBuffer[0].string = NULL;
      if      (NArgs==0) ((void(*)(value*,                            int*,char*))FuncDefn->FunctionPtr)(                                                            ResultBuffer,&j,errtext);
      else if (NArgs==1) ((void(*)(value*,value*,                     int*,char*))FuncDefn->FunctionPtr)(                                             ResultBuffer+2,ResultBuffer,&j,errtext);
      else if (NArgs==2) ((void(*)(value*,value*,value*,              int*,char*))FuncDefn->FunctionPtr)(                              ResultBuffer+2,ResultBuffer+3,ResultBuffer,&j,errtext);
      else if (NArgs==3) ((void(*)(value*,value*,value*,value*,       int*,char*))FuncDefn->FunctionPtr)(               ResultBuffer+2,ResultBuffer+3,ResultBuffer+4,ResultBuffer,&j,errtext);
      else if (NArgs==4) ((void(*)(value*,value*,value*,value*,value*,int*,char*))FuncDefn->FunctionPtr)(ResultBuffer+2,ResultBuffer+3,ResultBuffer+4,ResultBuffer+5,ResultBuffer,&j,errtext);
      if (j>0) { *errpos = start; return; }
      if ((end!=NULL)&&(*end<0)) *end=pos;
      strcpy(out, ResultBuffer[0].string);
      return;
     }
    else
     {
      if ((end!=NULL)&&(*end>0)&&(pos<*end)) { *errpos = pos; strcpy(errtext, "Syntax Error: Unexpected trailing matter after variable name."); return; } // Have we used up as many characters as we were told we had to?
      DictLookup(_ppl_UserSpace_Vars, FormatString, NULL, (void **)&VarData); // Look up in database of variable definitions
      if ((VarData == NULL) || (VarData->modified==2)) { *errpos = start; sprintf(errtext, "No such variable, '%s'.", FormatString); return; }
      if (VarData->string == NULL) { *errpos = start; strcpy(errtext, "Type Error: This is a numeric variable where a string is expected."); return; }
      *errpos = -1;
      if ((end!=NULL)&&(*end<0)) *end=pos;
      strcpy(out, VarData->string);
      return;
     }
   }

  // We have a quoted string
  QuoteType = in[pos++];
  for (; ((in[pos]!='\0') && (in[pos]!=QuoteType) && ((end==NULL)||(*end<0)||(pos<*end))); pos++)
   {
    if ((in[pos]=='\\') && (in[pos+1]=='\\')) { FormatString[outpos++]='\\'; pos++; continue; }
    if ((in[pos]=='\\') && (in[pos+1]=='\'')) { FormatString[outpos++]='\''; pos++; continue; }
    if ((in[pos]=='\\') && (in[pos+1]=='\"')) { FormatString[outpos++]='\"'; pos++; continue; }
    FormatString[outpos++] = in[pos];
   }
  FormatString[outpos] = '\0';
  if (in[pos]!=QuoteType) { *errpos = start; strcpy(errtext, "Syntax Error: Mismatched quote"); return; }
  while ((in[++pos]>'\0') && (in[pos]<=' ')); // Fast-forward over trailing spaces

  // We have a quoted string with no % operator after it
  if (in[pos]!='%')
   {
    if ((end!=NULL)&&(*end>0)&&(pos<*end)) { *errpos = pos; strcpy(errtext, "Syntax Error: Unexpected trailing matter after quoted string"); return; } // Have we used up as many characters as we were told we had to?
    *errpos = -1;
    if ((end!=NULL)&&(*end<0)) *end=pos;
    strcpy(out, FormatString);
    return;
   }

  // We have read a format string, but have a % operator acting upon it
  pos++; // Skip over % character
  while ((in[pos]>'\0') && (in[pos]<=' ')) pos++; // Fast-forward over preceding spaces
  if (in[pos]!='(') { *errpos = pos; strcpy(errtext, "Syntax Error: An opening bracket was expected here"); return; }
  StrBracketMatch(in+pos, CommaPositions, &NFormatItems, &pos2, MAX_STR_FORMAT_ITEMS);
  if (pos2<0) { *errpos = pos; strcpy(errtext, "Syntax Error: Mismatched bracket"); return; }
  if (NFormatItems>=MAX_STR_FORMAT_ITEMS) { *errpos = pos; strcpy(errtext, "Overflow Error: Too many string formatting arguments"); return; }
  if ((end!=NULL)&&(*end>0)&&(pos+pos2<*end))
       { *errpos = pos; strcpy(errtext, "Syntax Error: Unexpected trailing matter after quoted string"); return; } // Have we used up as many characters as we were told we had to?

  // We have now read in a format string, and a bracketed list of format arguments
  // See <http://www.cplusplus.com/reference/clibrary/cstdio/sprintf/>
  for (i=j=ArgCount=0; FormatString[i]!='\0'; i++)
   {
    if (FormatString[i]!='%') { out[j++] = FormatString[i]; continue; }
    k=i+1; // k looks ahead to see experimentally if syntax is right
    RequiredArgs =1; // Normal %f like tokens require 1 argument
    if ((FormatString[k]=='-')||(FormatString[k]=='-')||(FormatString[k]==' ')||(FormatString[k]=='#')) k++; // optional flag can be <+- #>
    if (FormatString[k]=='*') { k++; RequiredArgs++; }
    else while ((FormatString[k]>='0') && (FormatString[k]<='9')) k++; // length can be * or some digits
    if (FormatString[k]=='.') // precision starts with a . and is followed by * or by digits
     {
      k++;
      if (FormatString[k]=='*') { k++; RequiredArgs++; }
      else while ((FormatString[k]>='0') && (FormatString[k]<='9')) k++; // length can be * or some digits
     }
    // We do not allow user to specify optional length flag, which could potentially be <hlL>

    for (l=0; AllowedFormats[l]!='\0'; l++) if (FormatString[k]==AllowedFormats[l]) break;
    if ((AllowedFormats[l]=='\0') || ((AllowedFormats[l]=='%')&&(FormatString[k-1]!='%')) || (RequiredArgs > NFormatItems-ArgCount))
     { out[j++] = FormatString[i]; continue; } // Have not got correct syntax for a format specifier

    if (AllowedFormats[l]=='%')
     {
      strcpy(out+j, "%"); // %% just produces a % character
     } else {
      StrSlice(FormatString, FormatToken, i, k+1);
      if (RequiredArgs>1) // Length and/or precision was specified with a *, so we need to take an integer from the list of arguments
       {
        ppl_EvaluateAlgebra(in+pos , &arg1v, CommaPositions[ArgCount+0]+1, &CommaPositions[ArgCount+1], errpos, errtext, RecursionDepth+1);
        if (*errpos>=0) { *errpos += pos; return; }
        if (arg1v.dimensionless == 0) { *errpos = pos; sprintf(errtext, "This argument should be dimensionless, but has dimensions of <%s>.", ppl_units_GetUnitStr(&arg1v, NULL, NULL, 0, 0) ); return; }
        if (arg1v.FlagComplex       ) { *errpos = pos; sprintf(errtext, "This argument should real, but in fact has an imaginary component."); return; }
        arg1i = (int)(arg1v.real);
       }
      if (RequiredArgs>2)
       {
        ppl_EvaluateAlgebra(in+pos , &arg2v, CommaPositions[ArgCount+1]+1, &CommaPositions[ArgCount+2], errpos, errtext, RecursionDepth+1);
        if (*errpos>=0) { *errpos += pos; return; }
        if (arg2v.dimensionless == 0) { *errpos = pos; sprintf(errtext, "This argument should be dimensionless, but has dimensions of <%s>.", ppl_units_GetUnitStr(&arg2v, NULL, NULL, 0, 0) ); return; }
        if (arg2v.FlagComplex       ) { *errpos = pos; sprintf(errtext, "This argument should real, but in fact has an imaginary component."); return; }
        arg2i = (int)(arg2v.real);
       }

      ArgCount += (RequiredArgs-1); // Fastforward ArgCount to point at the number or string argument that we're actually going to substitute
      if ((AllowedFormats[l]=='c') || (AllowedFormats[l]=='s') || (AllowedFormats[l]=='S'))
       {
        ppl_EvaluateAlgebra(in+pos , &argtemp, CommaPositions[ArgCount+0]+1, &CommaPositions[ArgCount+1], errpos, errtext, RecursionDepth+1);
        if (*errpos>=0)
         { 
          *errpos = -1;
          ppl_GetQuotedString(in+pos , FormatArg , CommaPositions[ArgCount+0]+1, &CommaPositions[ArgCount+1], errpos, errtext, RecursionDepth+1);
          if (*errpos>=0) { *errpos += pos; return; } // Error from attempt at string evaluation should prevail
          argf.s = FormatArg;
         } else {
          argf.s = ppl_units_NumericDisplay(&argtemp, 0, 0);
         }
        if (RequiredArgs==1) sprintf(out+j, FormatToken, argf.s); // Print a string variable
        if (RequiredArgs==2) sprintf(out+j, FormatToken, arg1i, argf.s);
        if (RequiredArgs==3) sprintf(out+j, FormatToken, arg1i, arg2i, argf.s);
       }
      else
       {
        ppl_EvaluateAlgebra(in+pos , &argf.v, CommaPositions[ArgCount+0]+1, &CommaPositions[ArgCount+1], errpos, errtext, RecursionDepth+1);
        if (*errpos>=0) { *errpos += pos; return; }
        if ((settings_term_current.ComplexNumbers == SW_ONOFF_OFF) && (argf.v.FlagComplex!=0))
         { sprintf(out+j, "nan"); }
        else
         {
          argf.s = ppl_units_GetUnitStr(&argf.v, &argf.d, &argf.d2, 0, 0);
          if      ((AllowedFormats[l]=='d') || (AllowedFormats[l]=='i') || (AllowedFormats[l]=='o') || (AllowedFormats[l]=='x') || (AllowedFormats[l]=='X'))
           {
            if ((argf.d<=INT_MIN)||(argf.d>=INT_MAX)||(!gsl_finite(argf.d))||(argf.d2<=INT_MIN)||(argf.d2>=INT_MAX)||(!gsl_finite(argf.d2)))
             {
              *errpos = pos; sprintf(errtext, "This argument is too large to represent as an integer.");
             }
            else if (argf.v.FlagComplex == 0) // Print a real integer
             {
              argf.i = (int)argf.d; // sprintf will expect to be passed an int
              if (RequiredArgs==1) sprintf(out+j, FormatToken, argf.i); // Print an integer variable
              if (RequiredArgs==2) sprintf(out+j, FormatToken, arg1i, argf.i);
              if (RequiredArgs==3) sprintf(out+j, FormatToken, arg1i, arg2i, argf.i);
             }
            else // Print a complex integer
             {
              argf.i = (int)argf.d ; // sprintf will expect to be passed an int real part, and an int imag part
              argf.i2= (int)argf.d2;
              if (RequiredArgs==1) sprintf(out+j, FormatToken, argf.i); // Print an integer real part
              if (RequiredArgs==2) sprintf(out+j, FormatToken, arg1i, argf.i);
              if (RequiredArgs==3) sprintf(out+j, FormatToken, arg1i, arg2i, argf.i);
              j += strlen(out+j);
              out[j++]='+';
              if (RequiredArgs==1) sprintf(out+j, FormatToken, argf.i2); // Print an integer imag part
              if (RequiredArgs==2) sprintf(out+j, FormatToken, arg1i, argf.i2);
              if (RequiredArgs==3) sprintf(out+j, FormatToken, arg1i, arg2i, argf.i2);
              j += strlen(out+j);
              strcpy(out+j,"i");
             }
           }
          else if ((AllowedFormats[l]=='x') || (AllowedFormats[l]=='X'))
           {
            if ((argf.d<0)||(argf.d>=UINT_MAX)||(!gsl_finite(argf.d))||(argf.d2<0)||(argf.d2>=UINT_MAX)||(!gsl_finite(argf.d2)))
             {
              *errpos = pos; sprintf(errtext, "This argument is outside the range which can be represented as an unsigned integer.");
             }
            else if (argf.v.FlagComplex == 0) // Print a real unsigned integer
             {
              argf.u = (unsigned int)argf.d; // sprintf will expect to be passed an unsigned int
              if (RequiredArgs==1) sprintf(out+j, FormatToken, argf.u); // Print an integer variable
              if (RequiredArgs==2) sprintf(out+j, FormatToken, arg1i, argf.u);
              if (RequiredArgs==3) sprintf(out+j, FormatToken, arg1i, arg2i, argf.u);
             }
            else // Print a complex unsigned integer
             {
              argf.u = (unsigned int)argf.d ; // sprintf will expect to be passed an int real part, and an int imag part
              argf.u2= (unsigned int)argf.d2;
              if (RequiredArgs==1) sprintf(out+j, FormatToken, argf.u); // Print an integer real part
              if (RequiredArgs==2) sprintf(out+j, FormatToken, arg1i, argf.u);
              if (RequiredArgs==3) sprintf(out+j, FormatToken, arg1i, arg2i, argf.u);
              j += strlen(out+j);
              out[j++]='+';
              if (RequiredArgs==1) sprintf(out+j, FormatToken, argf.u2); // Print an integer imag part
              if (RequiredArgs==2) sprintf(out+j, FormatToken, arg1i, argf.u2);
              if (RequiredArgs==3) sprintf(out+j, FormatToken, arg1i, arg2i, argf.u2);
              j += strlen(out+j);
              strcpy(out+j,"i");
             }
           }
          else // otherwise, sprintf will expect a double
           {
            if (RequiredArgs==1) sprintf(out+j, FormatToken, argf.d); // Print a double (real part)
            if (RequiredArgs==2) sprintf(out+j, FormatToken, arg1i, argf.d);
            if (RequiredArgs==3) sprintf(out+j, FormatToken, arg1i, arg2i, argf.d);
            if (argf.v.FlagComplex != 0) // Print a complex double
             {
              j += strlen(out+j);
              out[j++]='+';
              if (RequiredArgs==1) sprintf(out+j, FormatToken, argf.d2); // Print the imaginary part
              if (RequiredArgs==2) sprintf(out+j, FormatToken, arg1i, argf.d2);
              if (RequiredArgs==3) sprintf(out+j, FormatToken, arg1i, arg2i, argf.d2);
              j += strlen(out+j);
              strcpy(out+j,"i");
             }
           }
          if (argf.v.dimensionless != 0)
           {
            j += strlen(out+j);
            strcpy(out+j, argf.s); // Print dimensions of this value
           }
         }
       }
      ArgCount++;
     }
    j += strlen(out+j);
    i = k;
   }
  out[j]  = '\0';
  *errpos = -1;
  if ((end!=NULL)&&(*end<0)) *end=pos+pos2+1;
  return;
 }

// ---------------------------------------------
// Functions for analysing algebraic expressions
// ---------------------------------------------

// ppl_EvaluateAlgebra(): Evaluates an algebraic expression to return a double. All fields as above.

// Order of precedence:
//  1 f(x)
//  2 ()
//    [evaluate numerics and variables]
//  3 minus signs
//  4 **
//  5 *  /  %
//  6 +  -
//  7 <<  >>
//  8 <  <=  >=  >
//  9 ==  !=  <>
// 10 &
// 11 ^
// 12 |
// 13 and
// 14 or

// StatusRow values 20-255 correspond to ResultBuffer values 0-235
#define BUFFER_OFFSET 20

#define MATCH_ONE(A)       (in[i]==A)
#define MATCH_TWO(A,B)    ((in[i]==A)&&(in[i+1]==B))
#define MATCH_THR(A,B,C)  ((in[i]==A)&&(in[i+1]==B)&&(in[i+2]==C))

#define FETCHPREV(VARA,VARB,VARC)  { for (j=p-1,cj=StatusRow[p-1]; ((StatusRow[j]==cj)&&(j>0)); j--); if (cj<BUFFER_OFFSET) {*errpos=i; strcpy(errtext, "Internal Error: Trying to do arithmetic before numeric conversion"); return;}; VARA=j; VARB=(int)(cj-BUFFER_OFFSET); VARC=p-1; }
#define FETCHNEXT(VARA,VARB,VARC)  { for (j=p,ci=StatusRow[p]; StatusRow[j]==ci; j++); if ((cj=StatusRow[j])<BUFFER_OFFSET) {*errpos=i; strcpy(errtext, "Internal Error: Trying to do arithmetic before numeric conversion"); return;}; for (k=j; StatusRow[k]==cj; k++); VARA=j; VARB=(int)(cj-BUFFER_OFFSET); VARC=k; }
#define SETSTATUS(BEG,END,VAL)     { ci = (unsigned char)(VAL+BUFFER_OFFSET); for (j=BEG;j<END;j++) StatusRow[j]=ci; }

void ppl_EvaluateAlgebra(char *in, value *out, int start, int *end, int *errpos, char *errtext, int RecursionDepth)
 {
  unsigned char OpList[OPLIST_LEN];           // A list of what operations this expression contains
  unsigned char StatusRow[ALGEBRA_MAXLENGTH]; // Describes the atoms at each position in the expression
  value ResultBuffer[ALGEBRA_MAXITEMS];       // A buffer of temporary numerical results
  int len, CalculatedEnd;
  int i,p,j,k,l,FunctionType, NArgs;
  int prev_start, prev_end, next_start, next_end, prev_bufno, next_bufno;
  unsigned char ci,cj;
  char dummy[DUMMYVAR_MAXLEN];
  char ck, *integrand;
  int bufpos = 0;
  value  *VarData  = NULL;
  FunctionDescriptor *FuncDef = NULL;
  double  TempDbl, TempDbl2;
  DictIterator *DictIter;

  if (RecursionDepth > MAX_RECURSION_DEPTH) { *errpos=start; strcpy(errtext,"Overflow Error: Maximum recursion depth exceeded"); return; }

  *errpos = -1;
  ppl_GetExpression(in+start, &len, 0, StatusRow, OpList, errpos, errtext);
  if (*errpos >= 0) { (*errpos) += start; return; }
  CalculatedEnd = start + len;
  if ((end != NULL) && (*end >  0) && (CalculatedEnd < *end)) { *errpos=CalculatedEnd; strcpy(errtext,"Syntax Error: Unexpected trailing matter after algebraic expression"); return; }
  if ((end != NULL) && (*end <= 0)) *end = CalculatedEnd;

  // PHASE  1: EVALUATION OF FUNCTIONS
  if (OpList[1]!=0) for (i=0;i<len-1;i++) if ((StatusRow[i]==8)&&(StatusRow[i+1]==3))
   {
    for (j=i;((j>0)&&(StatusRow[j-1]==8));j--); // Rewind to beginning of function name
    p=0;
    for ( DictIter=DictIterateInit(_ppl_UserSpace_Funcs) ; ((DictIter!=NULL)&&(p==0)) ; DictIter=DictIterate(DictIter,NULL,NULL) )
     {
      for (k=0; ((DictIter->key[k]>' ')&&(DictIter->key[k]!='?')&&(DictIter->key[k]==in[start+j+k])); k++); // See if string we have matches the name of this function
      if (DictIter->key[k]=='?') // This function name, e.g. int_dx, ends with a dummy variable name
       {
        for (l=0; ((isalnum(in[start+j+k+l]) || (in[start+j+k+l]=='_')) && (l<DUMMYVAR_MAXLEN)); l++) dummy[l]=in[start+j+k+l];
        if (l==DUMMYVAR_MAXLEN) continue; // Dummy variable name was too long
        dummy[l]='\0';
       } else { // Otherwise, we have to match function name exactly
        if ((DictIter->key[k]>' ') || (isalnum(in[start+j+k])) || (in[start+j+k]=='_')) continue; // Nope...
       }
      p=1; i+=2;
      NArgs = ((FunctionDescriptor *)DictIter->data)->NumberArguments;
      FunctionType = ((FunctionDescriptor *)DictIter->data)->FunctionType;
      for (k=0; k<NArgs; k++) // Now collect together numeric arguments
       {
        if (bufpos+k+2 >= ALGEBRA_MAXITEMS) { *errpos = start+i; strcpy(errtext,"Internal error: Temporary results buffer overflow."); return; }
        while ((in[start+i]>'\0')&&(in[start+i]<=' ')) i++;
        if (in[start+i]==')') { *errpos = start+i; strcpy(errtext,"Syntax Error: Too few arguments supplied to function."); return; }
        j=-1;
        if ((k==0)&&(FunctionType == PPL_USERSPACE_INT)) // First argument is an integrand, which we should not evaluate
         {
          if ((in[start+i]=='\'') || (in[start+i]=='\"')) { ck=in[start+i]; i++; } else { ck='\0'; }
          while ((in[start+i]>'\0')&&(in[start+i]<=' ')) i++;
          integrand = in+start+i; // Integrand starts here
          ppl_GetExpression(in+start+i, &j, 0, NULL, NULL, errpos, errtext);
          if (*errpos >= 0) { (*errpos) += start+i; return; }
          i+=j;
          while ((in[start+i]>'\0')&&(in[start+i]<=' ')) i++;
          if (ck>'\0')
           {
            if (in[start+i]!=ck) { *errpos = start+i; strcpy(errtext,"Syntax Error: Was expecting a closing quote."); return; }
            else                    i++;
           }
         } else { // otherwise, we evaluate the argument straight away
          ppl_EvaluateAlgebra(in+start+i, ResultBuffer+bufpos+k+2, 0, &j, errpos, errtext, RecursionDepth+1);
          if (*errpos >= 0) { (*errpos) += start+i; return; }
          i+=j;
         }
        while ((in[start+i]>'\0')&&(in[start+i]<=' ')) i++;
        if (k < NArgs-1)
         {
          if (in[start+i] != ',')
           {
            *errpos = start+i;
            if (in[start+i] ==')') strcpy(errtext,"Syntax Error: Too few arguments supplied to function.");
            else                   strcpy(errtext,"Syntax Error: Unexpected trailing matter after argument to function.");
            return;
           } else { i++; }
         }
       }
      if (FunctionType == PPL_USERSPACE_STRFUNC)
       {
        while (StatusRow[i]==3) i--; while ((i>0)&&(StatusRow[i]==8)) i--; if (StatusRow[i]!=8) i++; // Rewind back to beginning of f(x) text
        (*errpos) = start+i;
        strcpy(errtext,"Type error: This function returns a string where a numeric result was expected.");
        return;
       }
      if ((FunctionType != PPL_USERSPACE_UNIT) && (in[start+i] != ')')) // Unit function deals with arguments itself
       {
        (*errpos) = start+i;
        if (in[start+i] ==',') strcpy(errtext,"Syntax Error: Too many arguments supplied to function.");
        else                   strcpy(errtext,"Syntax Error: Unexpected trailing matter after final argument to function.");
        return;
       }
      if (FunctionType == PPL_USERSPACE_SYSTEM)
       {
        while (StatusRow[i]==3) i--; while ((i>0)&&(StatusRow[i]==8)) i--; if (StatusRow[i]!=8) i++; // Rewind back to beginning of f(x) text
        j=0;
        if      (NArgs==0) ((void(*)(value*,                                          int*,char*))((FunctionDescriptor*)DictIter->data)->FunctionPtr)(                                                                                                                                    ResultBuffer+bufpos,&j,errtext);
        else if (NArgs==1) ((void(*)(value*,value*,                                   int*,char*))((FunctionDescriptor*)DictIter->data)->FunctionPtr)(                                                                                                              ResultBuffer+bufpos+2,ResultBuffer+bufpos,&j,errtext);
        else if (NArgs==2) ((void(*)(value*,value*,value*,                            int*,char*))((FunctionDescriptor*)DictIter->data)->FunctionPtr)(                                                                                        ResultBuffer+bufpos+2,ResultBuffer+bufpos+3,ResultBuffer+bufpos,&j,errtext);
        else if (NArgs==3) ((void(*)(value*,value*,value*,value*,                     int*,char*))((FunctionDescriptor*)DictIter->data)->FunctionPtr)(                                                                  ResultBuffer+bufpos+2,ResultBuffer+bufpos+3,ResultBuffer+bufpos+4,ResultBuffer+bufpos,&j,errtext);
        else if (NArgs==4) ((void(*)(value*,value*,value*,value*,value*,              int*,char*))((FunctionDescriptor*)DictIter->data)->FunctionPtr)(                                            ResultBuffer+bufpos+2,ResultBuffer+bufpos+3,ResultBuffer+bufpos+4,ResultBuffer+bufpos+5,ResultBuffer+bufpos,&j,errtext);
        else if (NArgs==5) ((void(*)(value*,value*,value*,value*,value*,value*,       int*,char*))((FunctionDescriptor*)DictIter->data)->FunctionPtr)(                      ResultBuffer+bufpos+2,ResultBuffer+bufpos+3,ResultBuffer+bufpos+4,ResultBuffer+bufpos+5,ResultBuffer+bufpos+6,ResultBuffer+bufpos,&j,errtext);
        else if (NArgs==6) ((void(*)(value*,value*,value*,value*,value*,value*,value*,int*,char*))((FunctionDescriptor*)DictIter->data)->FunctionPtr)(ResultBuffer+bufpos+2,ResultBuffer+bufpos+3,ResultBuffer+bufpos+4,ResultBuffer+bufpos+5,ResultBuffer+bufpos+6,ResultBuffer+bufpos+7,ResultBuffer+bufpos,&j,errtext);
        if (j>0) { *errpos = start+i; return; }
       }
      else if (FunctionType == PPL_USERSPACE_UNIT)
       {
        j=-1;
        ppl_units_StringEvaluate(in+start+i, ResultBuffer+bufpos, &k, &j, errtext);
        if (j>=0) { *errpos = start+i+j; return; }
        i+=k; while ((in[start+i]>'\0')&&(in[start+i]<=' ')) i++;
        if (in[start+i] != ')') { (*errpos) = start+i; strcpy(errtext,"Syntax Error: Unexpected trailing matter after unit expression."); return; }
        while (StatusRow[i]==3) i--; while ((i>0)&&(StatusRow[i]==8)) i--; if (StatusRow[i]!=8) i++;
       }
      else if (FunctionType == PPL_USERSPACE_USERDEF)
       {
        while (StatusRow[i]==3) i--; while ((i>0)&&(StatusRow[i]==8)) i--; if (StatusRow[i]!=8) i++;
        FuncDef = (FunctionDescriptor *)DictIter->data;
        j=0;
        while (FuncDef != NULL) // Check whether supplied arguments are within the range of this definition
         {
          l=1;
          for (k=0; ((k<NArgs)&&(l==1)); k++)
           {
            if (FuncDef->MinActive[k]!=0)
             {
              if (j==0)
               {
                if (!ppl_units_DimEqual(FuncDef->min+k , ResultBuffer+bufpos+k+2))
                 {
                  *errpos = start+i;
                  sprintf(errtext,"Argument %d supplied to this function is dimensionally incompatible with the argument's specified min/max range: argument has dimensions of <%s>, meanwhile range has dimensions of <%s>.",k+1,ppl_units_GetUnitStr(ResultBuffer+bufpos+k+2,NULL,NULL,0,0),ppl_units_GetUnitStr(FuncDef->min+k,NULL,NULL,1,0));
                  return;
                 }
                else if (ResultBuffer[bufpos+k+2].FlagComplex)
                 {
                  *errpos = start+i;
                  sprintf(errtext,"Argument %d supplied to this function must be a real number: any arguments which have min/max ranges specified must be real.",k+1);
                  return;
                 } else { j=1; }
               }
              if (ResultBuffer[bufpos+k+2].real < FuncDef->min[k].real) { FuncDef=FuncDef->next; l=0; continue; }
             }
            if (FuncDef->MaxActive[k]!=0)
             { 
              if (j==0)
               {
                if (!ppl_units_DimEqual(FuncDef->max+k , ResultBuffer+bufpos+k+2))
                 {
                  *errpos = start+i;
                  sprintf(errtext,"Argument %d supplied to this function is dimensionally incompatible with the argument's specified min/max range: argument has dimensions of <%s>, meanwhile range has dimensions of <%s>.",k+1,ppl_units_GetUnitStr(ResultBuffer+bufpos+k+2,NULL,NULL,0,0),ppl_units_GetUnitStr(FuncDef->max+k,NULL,NULL,1,0));
                  return;
                 } 
                else if (ResultBuffer[bufpos+k+2].FlagComplex)
                 {
                  *errpos = start+i;
                  sprintf(errtext,"Argument %d supplied to this function must be a real number: any arguments which have min/max ranges specified must be real.",k+1);
                  return;
                 } else { j=1; }
               }
              if (ResultBuffer[bufpos+k+2].real > FuncDef->max[k].real) { FuncDef=FuncDef->next; l=0; continue; }
             }
           }
          if (l==1) break;
         }
        if (FuncDef==NULL)
         {
          if (settings_term_current.ExplicitErrors == SW_ONOFF_OFF) { ppl_units_zero(ResultBuffer+bufpos); ResultBuffer[bufpos].real = GSL_NAN; ResultBuffer[bufpos].imag = 0; }
          else { *errpos = start; sprintf(errtext,"This function is not defined in the requested region of parameter space."); return; }
         } else {
          j=0;
          for (k=0; k<NArgs; k++) // Swap new arguments for old in global dictionary
           {
            DictLookup(_ppl_UserSpace_Vars, ((FunctionDescriptor *)DictIter->data)->ArgList+j, NULL, (void **)&VarData);
            if (VarData!=NULL)
             {
              memcpy(ResultBuffer+bufpos+k+1, VarData, sizeof(value));
              memcpy(VarData, ResultBuffer+bufpos+k+2, sizeof(value));
             }
            else
             {
              ppl_units_zero(ResultBuffer+bufpos+k+1);
              ResultBuffer[bufpos+k+1].modified=2;
              DictAppendValue(_ppl_UserSpace_Vars, ((FunctionDescriptor *)DictIter->data)->ArgList+j, ResultBuffer[bufpos+k+2]);
             }
            j += strlen(((FunctionDescriptor *)DictIter->data)->ArgList+j)+1;
           }
          j=-1;
          ppl_EvaluateAlgebra((char *)FuncDef->FunctionPtr, ResultBuffer+bufpos, 0, &j, errpos, errtext, RecursionDepth+1);
          if (((char *)FuncDef->FunctionPtr)[j]!='\0') { *errpos=1; strcpy(errtext,"Unexpected trailing matter in function definition."); }
          j=0;
          for (k=0; k<NArgs; k++) // Swap old arguments for new in global dictionary
           {
            DictLookup(_ppl_UserSpace_Vars, ((FunctionDescriptor *)DictIter->data)->ArgList+j, NULL, (void **)&VarData);
            memcpy(VarData, ResultBuffer+bufpos+k+1, sizeof(value));
            j += strlen(((FunctionDescriptor *)DictIter->data)->ArgList+j)+1;
           }
          if (*errpos >= 0) { (*errpos) = start+i; return; }
         }
       }
      else if (FunctionType == PPL_USERSPACE_INT)
       {
        while (StatusRow[i]==3) i--; while ((i>0)&&(StatusRow[i]==8)) i--; if (StatusRow[i]!=8) i++;
        if (DictIter->key[0]=='i') Integrate    ( integrand , dummy , ResultBuffer+bufpos+3 , ResultBuffer+bufpos+4 , ResultBuffer+bufpos , errpos , errtext , RecursionDepth );
        else                       Differentiate( integrand , dummy , ResultBuffer+bufpos+3 , ResultBuffer+bufpos+4 , ResultBuffer+bufpos , errpos , errtext , RecursionDepth );
        if (*errpos >= 0) { (*errpos) = start+i; return; }
       }
      for ( ; StatusRow[i]==8; i++) StatusRow[i] = (unsigned char)(bufpos + BUFFER_OFFSET);
      for ( ; StatusRow[i]==3; i++) StatusRow[i] = (unsigned char)(bufpos + BUFFER_OFFSET);
      bufpos++; if (bufpos >= ALGEBRA_MAXITEMS) { *errpos = start+i; strcpy(errtext,"Internal error: Temporary results buffer overflow."); return; }
     }
    if (p==0) { *errpos=start+j; strcpy(errtext,"No such function"); return; }
   }
  // PHASE  2: EVALUATION OF BRACKETED EXPRESSIONS
  if (OpList[2]!=0) for (i=0;i<len;i++) if (StatusRow[i]==3)
   {
    j=-1;
    ppl_EvaluateAlgebra(in+start+i, ResultBuffer+bufpos, 1, &j, errpos, errtext, RecursionDepth+1);
    if (*errpos >= 0) { (*errpos) += start+i; return; }
    j+=i; while ((in[start+j]>'\0')&&(in[start+j]<=' ')) j++;
    if (in[start+j]!=')') { *errpos=start+j; strcpy(errtext,"Syntax Error: Unexpected trailing matter within brackets."); return; }
    for (k=i; StatusRow[k]==3; k++) StatusRow[k] = (unsigned char)(bufpos + BUFFER_OFFSET);
    bufpos++; if (bufpos >= ALGEBRA_MAXITEMS) { *errpos = start+i; strcpy(errtext,"Internal error: Temporary results buffer overflow."); return; }
   }
  // PHASE 2b: EVALUATION OF ALL NUMERICS AND VARIABLES
  for (i=0;i<len;i++) if (StatusRow[i]==6)
   {
    ppl_units_zero(ResultBuffer+bufpos);
    ResultBuffer[bufpos].real = GetFloat(in+start+i, &j);
    for (k=i; StatusRow[k]==6; k++) StatusRow[k] = (unsigned char)(bufpos + BUFFER_OFFSET);
    j+=i; while ((in[start+j]>'\0')&&(in[start+j]<=' ')) j++;
    if (j!=k) { *errpos=start+i; strcpy(errtext,"Syntax Error: Unexpected trailing matter after numeric constant."); return; }
    bufpos++; if (bufpos >= ALGEBRA_MAXITEMS) { *errpos = start+i; strcpy(errtext,"Internal error: Temporary results buffer overflow."); return; }
   }
  for (i=0;i<len;i++) if ((StatusRow[i]==8) || (StatusRow[i]==4))
   {
    for (j=i;((StatusRow[j]==8)||(StatusRow[i]==4));j++);
    while ((j>i) && (in[start+j-1]<=' ')) j--;
    ck = in[start+j] ; in[start+j]='\0'; // This will not work if string constant is passed to us!!
    DictLookup(_ppl_UserSpace_Vars, in+start+i, NULL, (void **)&VarData);
    if ((VarData == NULL) || (VarData->modified==2)) { *errpos = start+i; sprintf(errtext, "No such variable, '%s'.", in+start+i); in[start+j] = ck; return; } 
    in[start+j] = ck;
    if (VarData->string != NULL) { *errpos = start+i; strcpy(errtext, "Type Error: This is a string variable where numeric value is expected."); return; }
    ResultBuffer[bufpos] = *VarData;
    for (k=i; ((StatusRow[k]==8)||(StatusRow[k]==4)); k++) StatusRow[k] = (unsigned char)(bufpos + BUFFER_OFFSET);
    bufpos++; if (bufpos >= ALGEBRA_MAXITEMS) { *errpos = start+i; strcpy(errtext,"Internal error: Temporary results buffer overflow."); return; }
   }
  // PHASE  3: EVALUATION OF MINUS SIGNS
  if (OpList[3]!=0) for (i=start,p=0;i<CalculatedEnd;i++,p++) if (StatusRow[p]==5)
   {
    FETCHNEXT(next_start, next_bufno, next_end);
    ResultBuffer[next_bufno].real *= -1;
    ResultBuffer[next_bufno].imag *= -1;
    SETSTATUS(p, next_start, next_bufno);
    i = start + next_start;
    i--; p=i-start;
   }
  // PHASE  4: EVALUATION OF **
  if (OpList[4]!=0) for (i=start,p=0;i<CalculatedEnd;i++,p++) if (StatusRow[p]==7)
   {
    if (MATCH_TWO('*','*'))
     {
      FETCHPREV(prev_start, prev_bufno, prev_end);
      FETCHNEXT(next_start, next_bufno, next_end);
      ppl_units_pow(ResultBuffer+prev_bufno , ResultBuffer+next_bufno , ResultBuffer+prev_bufno , errpos , errtext);
      if (*errpos >= 0) { *errpos=i; return; }
      SETSTATUS(prev_end, next_end, prev_bufno);
      i = start + next_start - 1; p=i-start;
     } else {
      while (StatusRow[p]==7) { i++; p++; }
     }
   }
  // PHASE  5: EVALUATION OF *  /  %
  if (OpList[5]!=0) for (i=start,p=0;i<CalculatedEnd;i++,p++) if (StatusRow[p]==7)
   {
    if ((MATCH_ONE('*'))||(MATCH_ONE('/'))||(MATCH_ONE('%')))
     {
      FETCHPREV(prev_start, prev_bufno, prev_end);
      FETCHNEXT(next_start, next_bufno, next_end);
      if      (MATCH_ONE('*')) ppl_units_mult(ResultBuffer+prev_bufno , ResultBuffer+next_bufno , ResultBuffer+prev_bufno , errpos , errtext);
      else if (MATCH_ONE('/')) ppl_units_div (ResultBuffer+prev_bufno , ResultBuffer+next_bufno , ResultBuffer+prev_bufno , errpos , errtext);
      else if (MATCH_ONE('%')) ppl_units_mod (ResultBuffer+prev_bufno , ResultBuffer+next_bufno , ResultBuffer+prev_bufno , errpos , errtext);
      if (*errpos >= 0) { *errpos=i; return; }
      SETSTATUS(prev_end, next_end, prev_bufno);
      i = start + next_start - 1; p=i-start;
     } else {
      while (StatusRow[p]==7) { i++; p++; }
     }
   }
  // PHASE  6: EVALUATION OF +  -
  if (OpList[6]!=0) for (i=start,p=0;i<CalculatedEnd;i++,p++) if (StatusRow[p]==7)
   {
    if ((MATCH_ONE('+'))||(MATCH_ONE('-')))
     {
      FETCHPREV(prev_start, prev_bufno, prev_end);
      FETCHNEXT(next_start, next_bufno, next_end);
      if      (MATCH_ONE('+')) ppl_units_add(ResultBuffer+prev_bufno , ResultBuffer+next_bufno , ResultBuffer+prev_bufno , errpos , errtext);
      else if (MATCH_ONE('-')) ppl_units_sub(ResultBuffer+prev_bufno , ResultBuffer+next_bufno , ResultBuffer+prev_bufno , errpos , errtext);
      if (*errpos >= 0) { *errpos=i; return; }
      SETSTATUS(prev_end, next_end, prev_bufno);
      i = start + next_start - 1; p=i-start;
     } else {
      while (StatusRow[p]==7) { i++; p++; }
     }
   }
  // PHASE  7: EVALUATION OF << >>
  if (OpList[7]!=0) for (i=start,p=0;i<CalculatedEnd;i++,p++) if (StatusRow[p]==7)
   {
    if ((MATCH_TWO('<','<'))||(MATCH_TWO('>','>')))
     {
      FETCHPREV(prev_start, prev_bufno, prev_end);
      FETCHNEXT(next_start, next_bufno, next_end);
      if ( (ResultBuffer[prev_bufno].dimensionless == 0) || (ResultBuffer[next_bufno].dimensionless == 0) )
       {
        if ( (ResultBuffer[prev_bufno].dimensionless == 0) && (ResultBuffer[next_bufno].dimensionless == 0) )
         { *errpos=i; sprintf(errtext, "Binary operators can only be applied to dimensionless operands; here, the left operand has units of <%s> and the right operand has units of <%s>.", ppl_units_GetUnitStr(ResultBuffer+prev_bufno, NULL, NULL, 0, 0), ppl_units_GetUnitStr(ResultBuffer+next_bufno, NULL, NULL, 1, 0)); return; }
        else if (ResultBuffer[prev_bufno].dimensionless == 0)
         { *errpos=i; sprintf(errtext, "Binary operators can only be applied to dimensionless operands; here, the left operand has units of <%s>.", ppl_units_GetUnitStr(ResultBuffer+prev_bufno, NULL, NULL, 0, 0) ); return; }
        else
         { *errpos=i; sprintf(errtext, "Binary operators can only be applied to dimensionless operands; here, the right operand has units of <%s>.", ppl_units_GetUnitStr(ResultBuffer+next_bufno, NULL, NULL, 0, 0) ); return; }
       }
      if (ResultBuffer[prev_bufno].FlagComplex || ResultBuffer[next_bufno].FlagComplex)
       { *errpos=i; sprintf(errtext, "Binary operators can only be applied to real operands; supplied operands have imaginary components."); return; }
      if ((ResultBuffer[prev_bufno].real<=INT_MIN)||(ResultBuffer[prev_bufno].real>=INT_MAX)||(!gsl_finite(ResultBuffer[prev_bufno].real))||(ResultBuffer[next_bufno].real<=INT_MIN)||(ResultBuffer[next_bufno].real>=INT_MAX)||(!gsl_finite(ResultBuffer[next_bufno].real)))
       { *errpos=i; sprintf(errtext, "Binary operators can only be applied to operands in the range %d<x<%d.",INT_MIN,INT_MAX); return; }
      if      (MATCH_ONE('<')) ResultBuffer[prev_bufno].real = (double)((int)ResultBuffer[prev_bufno].real << (int)ResultBuffer[next_bufno].real);
      else if (MATCH_ONE('>')) ResultBuffer[prev_bufno].real = (double)((int)ResultBuffer[prev_bufno].real >> (int)ResultBuffer[next_bufno].real);
      if ((ResultBuffer[prev_bufno].real<=INT_MIN)||(ResultBuffer[prev_bufno].real>=INT_MAX)||(!gsl_finite(ResultBuffer[prev_bufno].real)))
       { *errpos=i; sprintf(errtext, "Overflow error; the result of a binary operation must be in the range %d<x<%d.",INT_MIN,INT_MAX); return; }
      SETSTATUS(prev_end, next_end, prev_bufno);
      i = start + next_start - 1; p=i-start;
     } else {
      while (StatusRow[p]==7) { i++; p++; }
     }
   }
  // PHASE  8: EVALUATION OF < <= >= >
  if (OpList[8]!=0) for (i=start,p=0;i<CalculatedEnd;i++,p++) if (StatusRow[p]==7)
   {
    if ((MATCH_ONE('<'))||(MATCH_ONE('>')))
     {
      FETCHPREV(prev_start, prev_bufno, prev_end);
      FETCHNEXT(next_start, next_bufno, next_end);
      if (ppl_units_DimEqual(ResultBuffer+prev_bufno , ResultBuffer+next_bufno) == 0)
       {
        *errpos=i;
        if      (ResultBuffer[prev_bufno].dimensionless)
         { sprintf(errtext, "Attempt to compare a quantity which is dimensionless with one with dimensions of <%s>.", ppl_units_GetUnitStr(ResultBuffer+next_bufno,NULL,NULL,1,0)); }
        else if (ResultBuffer[next_bufno].dimensionless)
         { sprintf(errtext, "Attempt to compare a quantity with dimensions of <%s> with one which is dimensionless.", ppl_units_GetUnitStr(ResultBuffer+prev_bufno,NULL,NULL,0,0)); }
        else
         { sprintf(errtext, "Attempt to compare a quantity with dimensions of <%s> with one with dimensions of <%s>.", ppl_units_GetUnitStr(ResultBuffer+prev_bufno,NULL,NULL,0,0), ppl_units_GetUnitStr(ResultBuffer+next_bufno,NULL,NULL,1,0)); }
        return;
       }
      if (ResultBuffer[prev_bufno].FlagComplex || ResultBuffer[next_bufno].FlagComplex)
       { *errpos=i; sprintf(errtext, "Magnitude comparisons can only be applied to real operands; supplied operands have imaginary components."); return; }
      TempDbl = ResultBuffer[prev_bufno].real;
      ppl_units_zero(ResultBuffer+prev_bufno);
      if      (MATCH_TWO('<','=')) ResultBuffer[prev_bufno].real = (double)(TempDbl <= ResultBuffer[next_bufno].real);
      else if (MATCH_TWO('>','=')) ResultBuffer[prev_bufno].real = (double)(TempDbl >= ResultBuffer[next_bufno].real);
      else if (MATCH_ONE('<')    ) ResultBuffer[prev_bufno].real = (double)(TempDbl <  ResultBuffer[next_bufno].real);
      else if (MATCH_ONE('>')    ) ResultBuffer[prev_bufno].real = (double)(TempDbl >  ResultBuffer[next_bufno].real);
      SETSTATUS(prev_end, next_end, prev_bufno);
      i = start + next_start - 1; p=i-start;
     } else {
      while (StatusRow[p]==7) { i++; p++; }
     }
   }
  // PHASE  9: EVALUATION OF == !=  <>
  if (OpList[9]!=0) for (i=start,p=0;i<CalculatedEnd;i++,p++) if (StatusRow[p]==7)
   {
    if ((MATCH_TWO('=','='))||(MATCH_TWO('!','='))||(MATCH_TWO('<','>')))
     {
      FETCHPREV(prev_start, prev_bufno, prev_end);
      FETCHNEXT(next_start, next_bufno, next_end);
      if (ppl_units_DimEqual(ResultBuffer+prev_bufno , ResultBuffer+next_bufno) == 0)
      if (ppl_units_DimEqual(ResultBuffer+prev_bufno , ResultBuffer+next_bufno) == 0)
       { 
        *errpos=i; 
        if      (ResultBuffer[prev_bufno].dimensionless)
         { sprintf(errtext, "Attempt to compare a quantity which is dimensionless with one with dimensions of <%s>.", ppl_units_GetUnitStr(ResultBuffer+next_bufno,NULL,NULL,1,0)); }
        else if (ResultBuffer[next_bufno].dimensionless)
         { sprintf(errtext, "Attempt to compare a quantity with dimensions of <%s> with one which is dimensionless.", ppl_units_GetUnitStr(ResultBuffer+prev_bufno,NULL,NULL,0,0)); }
        else
         { sprintf(errtext, "Attempt to compare a quantity with dimensions of <%s> with one with dimensions of <%s>.", ppl_units_GetUnitStr(ResultBuffer+prev_bufno,NULL,NULL,0,0), ppl_units_GetUnitStr(ResultBuffer+next_bufno,NULL,NULL,1,0)); }
        return;
       }
      TempDbl = ResultBuffer[prev_bufno].real;
      TempDbl2= ResultBuffer[prev_bufno].imag;
      ppl_units_zero(ResultBuffer+prev_bufno);
      if      (MATCH_TWO('=','=')) ResultBuffer[prev_bufno].real = (double) (ppl_units_DblEqual(TempDbl, ResultBuffer[next_bufno].real) && ppl_units_DblEqual(TempDbl2, ResultBuffer[next_bufno].imag));
      else if (MATCH_TWO('!','=')) ResultBuffer[prev_bufno].real = (double)!(ppl_units_DblEqual(TempDbl, ResultBuffer[next_bufno].real) && ppl_units_DblEqual(TempDbl2, ResultBuffer[next_bufno].imag));
      else if (MATCH_TWO('<','>')) ResultBuffer[prev_bufno].real = (double)!(ppl_units_DblEqual(TempDbl, ResultBuffer[next_bufno].real) && ppl_units_DblEqual(TempDbl2, ResultBuffer[next_bufno].imag));
      SETSTATUS(prev_end, next_end, prev_bufno);
      i = start + next_start - 1; p=i-start;
     } else {
      while (StatusRow[p]==7) { i++; p++; }
     }
   }
  // PHASE 10: EVALUATION OF &
  if (OpList[10]!=0) for (i=start,p=0;i<CalculatedEnd;i++,p++) if (StatusRow[p]==7)
   {
    if (MATCH_ONE('&'))
     {
      FETCHPREV(prev_start, prev_bufno, prev_end);
      FETCHNEXT(next_start, next_bufno, next_end);
      if ( (ResultBuffer[prev_bufno].dimensionless == 0) || (ResultBuffer[next_bufno].dimensionless == 0) )
       {
        if ( (ResultBuffer[prev_bufno].dimensionless == 0) && (ResultBuffer[next_bufno].dimensionless == 0) )
         { *errpos=i; sprintf(errtext, "Binary operators can only be applied to dimensionless operands; here, the left operand has units of <%s> and the right operand has units of <%s>.", ppl_units_GetUnitStr(ResultBuffer+prev_bufno, NULL, NULL, 0, 0), ppl_units_GetUnitStr(ResultBuffer+next_bufno, NULL, NULL, 1, 0)); return; }
        else if (ResultBuffer[prev_bufno].dimensionless == 0)
         { *errpos=i; sprintf(errtext, "Binary operators can only be applied to dimensionless operands; here, the left operand has units of <%s>.", ppl_units_GetUnitStr(ResultBuffer+prev_bufno, NULL, NULL, 0, 0) ); return; }
        else
         { *errpos=i; sprintf(errtext, "Binary operators can only be applied to dimensionless operands; here, the right operand has units of <%s>.", ppl_units_GetUnitStr(ResultBuffer+next_bufno, NULL, NULL, 0, 0) ); return; }
       }
      if (ResultBuffer[prev_bufno].FlagComplex || ResultBuffer[next_bufno].FlagComplex)
       { *errpos=i; sprintf(errtext, "Binary operators can only be applied to real operands; supplied operands have imaginary components."); return; }
      if ((ResultBuffer[prev_bufno].real<=INT_MIN)||(ResultBuffer[prev_bufno].real>=INT_MAX)||(!gsl_finite(ResultBuffer[prev_bufno].real))||(ResultBuffer[next_bufno].real<=INT_MIN)||(ResultBuffer[next_bufno].real>=INT_MAX)||(!gsl_finite(ResultBuffer[next_bufno].real)))
       { *errpos=i; sprintf(errtext, "Binary operators can only be applied to operands in the range %d<x<%d.",INT_MIN,INT_MAX); return; }
      ResultBuffer[prev_bufno].real = (double)((int)ResultBuffer[prev_bufno].real & (int)ResultBuffer[next_bufno].real);
      SETSTATUS(prev_end, next_end, prev_bufno);
      i = start + next_start - 1; p=i-start;
     } else {
      while (StatusRow[p]==7) { i++; p++; }
     }
   }
  // PHASE 11: EVALUATION OF ^
  if (OpList[11]!=0) for (i=start,p=0;i<CalculatedEnd;i++,p++) if (StatusRow[p]==7)
   {
    if (MATCH_ONE('^'))
     {
      FETCHPREV(prev_start, prev_bufno, prev_end);
      FETCHNEXT(next_start, next_bufno, next_end);
      if ( (ResultBuffer[prev_bufno].dimensionless == 0) || (ResultBuffer[next_bufno].dimensionless == 0) )
       {
        if ( (ResultBuffer[prev_bufno].dimensionless == 0) && (ResultBuffer[next_bufno].dimensionless == 0) )
         { *errpos=i; sprintf(errtext, "Binary operators can only be applied to dimensionless operands; here, the left operand has units of <%s> and the right operand has units of <%s>.", ppl_units_GetUnitStr(ResultBuffer+prev_bufno, NULL, NULL, 0, 0), ppl_units_GetUnitStr(ResultBuffer+next_bufno, NULL, NULL, 1, 0)); return; }
        else if (ResultBuffer[prev_bufno].dimensionless == 0)
         { *errpos=i; sprintf(errtext, "Binary operators can only be applied to dimensionless operands; here, the left operand has units of <%s>.", ppl_units_GetUnitStr(ResultBuffer+prev_bufno, NULL, NULL, 0, 0) ); return; }
        else
         { *errpos=i; sprintf(errtext, "Binary operators can only be applied to dimensionless operands; here, the right operand has units of <%s>.", ppl_units_GetUnitStr(ResultBuffer+next_bufno, NULL, NULL, 0, 0) ); return; }
       }
      if (ResultBuffer[prev_bufno].FlagComplex || ResultBuffer[next_bufno].FlagComplex)
       { *errpos=i; sprintf(errtext, "Binary operators can only be applied to real operands; supplied operands have imaginary components."); return; }
      if ((ResultBuffer[prev_bufno].real<=INT_MIN)||(ResultBuffer[prev_bufno].real>=INT_MAX)||(!gsl_finite(ResultBuffer[prev_bufno].real))||(ResultBuffer[next_bufno].real<=INT_MIN)||(ResultBuffer[next_bufno].real>=INT_MAX)||(!gsl_finite(ResultBuffer[next_bufno].real)))
       { *errpos=i; sprintf(errtext, "Binary operators can only be applied to operands in the range %d<x<%d.",INT_MIN,INT_MAX); return; }
      ResultBuffer[prev_bufno].real = (double)((int)ResultBuffer[prev_bufno].real ^ (int)ResultBuffer[next_bufno].real);
      SETSTATUS(prev_end, next_end, prev_bufno);
      i = start + next_start - 1; p=i-start;
     } else {
      while (StatusRow[p]==7) { i++; p++; }
     }
   }
  // PHASE 12: EVALUATION OF |
  if (OpList[12]!=0) for (i=start,p=0;i<CalculatedEnd;i++,p++) if (StatusRow[p]==7)
   {
    if (MATCH_ONE('|'))
     {
      FETCHPREV(prev_start, prev_bufno, prev_end);
      FETCHNEXT(next_start, next_bufno, next_end);
      if ( (ResultBuffer[prev_bufno].dimensionless == 0) || (ResultBuffer[next_bufno].dimensionless == 0) )
       {
        if ( (ResultBuffer[prev_bufno].dimensionless == 0) && (ResultBuffer[next_bufno].dimensionless == 0) )
         { *errpos=i; sprintf(errtext, "Binary operators can only be applied to dimensionless operands; here, the left operand has units of <%s> and the right operand has units of <%s>.", ppl_units_GetUnitStr(ResultBuffer+prev_bufno, NULL, NULL, 0, 0), ppl_units_GetUnitStr(ResultBuffer+next_bufno, NULL, NULL, 1, 0)); return; }
        else if (ResultBuffer[prev_bufno].dimensionless == 0)
         { *errpos=i; sprintf(errtext, "Binary operators can only be applied to dimensionless operands; here, the left operand has units of <%s>.", ppl_units_GetUnitStr(ResultBuffer+prev_bufno, NULL, NULL, 0, 0) ); return; }
        else
         { *errpos=i; sprintf(errtext, "Binary operators can only be applied to dimensionless operands; here, the right operand has units of <%s>.", ppl_units_GetUnitStr(ResultBuffer+next_bufno, NULL, NULL, 0, 0) ); return; }
       }
      if (ResultBuffer[prev_bufno].FlagComplex || ResultBuffer[next_bufno].FlagComplex)
       { *errpos=i; sprintf(errtext, "Binary operators can only be applied to real operands; supplied operands have imaginary components."); return; }
      if ((ResultBuffer[prev_bufno].real<=INT_MIN)||(ResultBuffer[prev_bufno].real>=INT_MAX)||(!gsl_finite(ResultBuffer[prev_bufno].real))||(ResultBuffer[next_bufno].real<=INT_MIN)||(ResultBuffer[next_bufno].real>=INT_MAX)||(!gsl_finite(ResultBuffer[next_bufno].real)))
       { *errpos=i; sprintf(errtext, "Binary operators can only be applied to operands in the range %d<x<%d.",INT_MIN,INT_MAX); return; }
      ResultBuffer[prev_bufno].real = (double)((int)ResultBuffer[prev_bufno].real | (int)ResultBuffer[next_bufno].real);
      SETSTATUS(prev_end, next_end, prev_bufno);
      i = start + next_start - 1; p=i-start;
     } else {
      while (StatusRow[p]==7) { i++; p++; }
     }
   }
  // PHASE 13: EVALUATION OF and
  if (OpList[13]!=0) for (i=start,p=0;i<CalculatedEnd;i++,p++) if (StatusRow[p]==7)
   {
    if (MATCH_THR('a','n','d'))
     {
      FETCHPREV(prev_start, prev_bufno, prev_end);
      FETCHNEXT(next_start, next_bufno, next_end);
      if ( (ResultBuffer[prev_bufno].dimensionless == 0) || (ResultBuffer[next_bufno].dimensionless == 0) )
       {
        if ( (ResultBuffer[prev_bufno].dimensionless == 0) && (ResultBuffer[next_bufno].dimensionless == 0) )
         { *errpos=i; sprintf(errtext, "Logical operators can only be applied to dimensionless operands; here, the left operand has units of <%s> and the right operand has units of <%s>.", ppl_units_GetUnitStr(ResultBuffer+prev_bufno, NULL, NULL, 0, 0), ppl_units_GetUnitStr(ResultBuffer+next_bufno, NULL, NULL, 1, 0)); return; }
        else if (ResultBuffer[prev_bufno].dimensionless == 0)
         { *errpos=i; sprintf(errtext, "Logical operators can only be applied to dimensionless operands; here, the left operand has units of <%s>.", ppl_units_GetUnitStr(ResultBuffer+prev_bufno, NULL, NULL, 0, 0) ); return; }
        else
         { *errpos=i; sprintf(errtext, "Logical operators can only be applied to dimensionless operands; here, the right operand has units of <%s>.", ppl_units_GetUnitStr(ResultBuffer+next_bufno, NULL, NULL, 0, 0) ); return; }
       }
      if (ResultBuffer[prev_bufno].FlagComplex || ResultBuffer[next_bufno].FlagComplex)
       { *errpos=i; sprintf(errtext, "Logical operators can only be applied to real operands; supplied operands have imaginary components."); return; }
      if ((ResultBuffer[prev_bufno].real<=INT_MIN)||(ResultBuffer[prev_bufno].real>=INT_MAX)||(!gsl_finite(ResultBuffer[prev_bufno].real))||(ResultBuffer[next_bufno].real<=INT_MIN)||(ResultBuffer[next_bufno].real>=INT_MAX)||(!gsl_finite(ResultBuffer[next_bufno].real)))
       { *errpos=i; sprintf(errtext, "Logical operators can only be applied to operands in the range %d<x<%d.",INT_MIN,INT_MAX); return; }
      ResultBuffer[prev_bufno].real = (double)((int)ResultBuffer[prev_bufno].real && (int)ResultBuffer[next_bufno].real);
      SETSTATUS(prev_end, next_end, prev_bufno);
      i = start + next_start - 1; p=i-start;
     } else {
      while (StatusRow[p]==7) { i++; p++; }
     }
   }
  // PHASE 14: EVALUATION OF or
  if (OpList[14]!=0) for (i=start,p=0;i<CalculatedEnd;i++,p++) if (StatusRow[p]==7)
   {
    if (MATCH_TWO('o','r'))
     {
      FETCHPREV(prev_start, prev_bufno, prev_end);
      FETCHNEXT(next_start, next_bufno, next_end);
      if ( (ResultBuffer[prev_bufno].dimensionless == 0) || (ResultBuffer[next_bufno].dimensionless == 0) )
       {
        if ( (ResultBuffer[prev_bufno].dimensionless == 0) && (ResultBuffer[next_bufno].dimensionless == 0) )
         { *errpos=i; sprintf(errtext, "Logical operators can only be applied to dimensionless operands; here, the left operand has units of <%s> and the right operand has units of <%s>.", ppl_units_GetUnitStr(ResultBuffer+prev_bufno, NULL, NULL, 0, 0), ppl_units_GetUnitStr(ResultBuffer+next_bufno, NULL, NULL, 1, 0)); return; }
        else if (ResultBuffer[prev_bufno].dimensionless == 0)
         { *errpos=i; sprintf(errtext, "Logical operators can only be applied to dimensionless operands; here, the left operand has units of <%s>.", ppl_units_GetUnitStr(ResultBuffer+prev_bufno, NULL, NULL, 0, 0) ); return; }
        else
         { *errpos=i; sprintf(errtext, "Logical operators can only be applied to dimensionless operands; here, the right operand has units of <%s>.", ppl_units_GetUnitStr(ResultBuffer+next_bufno, NULL, NULL, 0, 0) ); return; }
       }
      if (ResultBuffer[prev_bufno].FlagComplex || ResultBuffer[next_bufno].FlagComplex)
       { *errpos=i; sprintf(errtext, "Logical operators can only be applied to real operands; supplied operands have imaginary components."); return; }
      if ((ResultBuffer[prev_bufno].real<=INT_MIN)||(ResultBuffer[prev_bufno].real>=INT_MAX)||(!gsl_finite(ResultBuffer[prev_bufno].real))||(ResultBuffer[next_bufno].real<=INT_MIN)||(ResultBuffer[next_bufno].real>=INT_MAX)||(!gsl_finite(ResultBuffer[next_bufno].real)))
       { *errpos=i; sprintf(errtext, "Logical operators can only be applied to operands in the range %d<x<%d.",INT_MIN,INT_MAX); return; }
      ResultBuffer[prev_bufno].real = (double)((int)ResultBuffer[prev_bufno].real || (int)ResultBuffer[next_bufno].real);
      SETSTATUS(prev_end, next_end, prev_bufno);
      i = start + next_start - 1; p=i-start;
     } else {
      while (StatusRow[p]==7) { i++; p++; }
     }
   }
  // PHASE 15: RETURN RESULT TO USER
  for (i=0;i<len;i++) if (StatusRow[i] >= BUFFER_OFFSET) { *out = ResultBuffer[ StatusRow[i] - BUFFER_OFFSET ]; return; }
  // for (i=0;i<len;i++) printf("%c",(StatusRow[i]+'A')); printf("\n"); // DEBUG LINE
  *errpos = start;
  strcpy(errtext,"Internal Error: ppl_EvaluateAlgebra failed to evaluate this expression"); // Oops
  return;
 }

// ppl_GetExpression(): Extracts a syntactically complete algebraic expression.

// end -- This is an output, and returns the end of the algebraic expression found
// dollar_allowed -- Indicates whether a dollar-sign is an allowable character in variable names
// status -- If non-NULL, the atom found at each character position is written here

// Go through string, spotting atoms from the following list:

// 1 S -- the beginning of the string
// 2 E -- the end of the expression
// 3 B -- a bracketed () series of characters
// 4 D -- a dollar sign -- only allowed in using expressions in the plot command as special variable name
// 5 M -- a minus sign before a numeric value, variable name, or ()
// 6 N -- a numerical value, e.g. 1.2e-34
// 7 O -- an operator, + - * ** / % << >> & | ^ < > <= >= == != <>
// 8 V -- a variable name

// 1 S can be followed by  BDMN V not by E    O
// 2 E
// 3 B can be followed by E    O  not by  BDMN V
// 4 D can be followed by  B  N V not by E DM O
// 5 M can be followed by  BD N V not by E  M O
// 6 N can be followed by E    O  not by  BDMN V
// 7 O can be followed by  BDMN V not by E    O
// 8 V can be followed by EB   O  not by   DMN V

#define MATCH_TRACK_ONE(A,OPN)      ((in[scanpos]==A)                                                                    && (OpList[OPN]=1,1))
#define MATCH_TRACK_TWO(A,B,OPN)    ((in[scanpos]==A)&&(in[scanpos+1]==B)                                                && (OpList[OPN]=1,1))
#define MATCH_TRACK_THR(A,B,C,OPN)  ((in[scanpos]==A)&&(in[scanpos+1]==B)&&(in[scanpos+2]==C)                            && (OpList[OPN]=1,1))
#define MATCH_TRACK_THRS(A,B,C,OPN) ((in[scanpos]==A)&&(in[scanpos+1]==B)&&(in[scanpos+2]==C)&&(!isalpha(in[scanpos+3])) && (OpList[OPN]=1,1))

#define PGE_OVERFLOW { *errpos = scanpos; strcpy(errtext, "Overflow Error: Algebraic expression too long"); *end = -1; return; }

#define NEWSTATE(L) state=trial; for (i=0;i<L;i++) { status[scanpos++]=(unsigned char)(state-'0') ; if (scanpos>=ALGEBRA_MAXLENGTH) PGE_OVERFLOW; }
#define SAMESTATE                                  { status[scanpos++]=(unsigned char)(state-'0') ; if (scanpos>=ALGEBRA_MAXLENGTH) PGE_OVERFLOW; }

void ppl_GetExpression(const char *in, int *end, int DollarAllowed, unsigned char *status, unsigned char *OpList, int *errpos, char *errtext)
 {
  static char *AllowedNext[] = {"34568","","72","368","34568","72","34568","372"};
  static unsigned char DummyOpList[OPLIST_LEN];
  static unsigned char DummyStatusRow[ALGEBRA_MAXLENGTH];
  char state='1', oldstate;
  char trial;
  int scanpos=0, trialpos, i, j; // scanpos scans through in.

  if (OpList==NULL) OpList = DummyOpList; // OpList is a table of which operations we've found, for speeding up evaluation. If this output is not requested, we put it in a dummy.
  if (status==NULL) status = DummyStatusRow;
  for (i=0;i<OPLIST_LEN;i++) OpList[i]=0;

  while (state != '2')
   {
    oldstate = state;
    while ((in[scanpos]!='\0') && (in[scanpos]<=' ')) { SAMESTATE; } // Sop up whitespace

    for (trialpos=0; ((trial=AllowedNext[(int)(state-'1')][trialpos])!='\0'); trialpos++)
     {
      if      (trial=='2') // E
       { NEWSTATE(0); }
      else if ((trial=='3') && MATCH_TRACK_ONE('(', 0)) // B
       {
        if (state=='8') OpList[1]=1; else OpList[2]=1; // We either have a bracketed expression, or a function call
        StrBracketMatch(in+scanpos, NULL, NULL, &j, 0);
        if (j>0) { j++; NEWSTATE(j); }
        else     { *errpos = scanpos; strcpy(errtext, "Syntax Error: Mismatched bracket"); *end = -1; return; }
       }
      else if ((trial=='4') && (DollarAllowed != 0) && MATCH_TRACK_ONE('$', 0)) // D
       { NEWSTATE(1); }
      else if (trial=='5') // M
       {
        if      (MATCH_TRACK_ONE ('-', 3))         { NEWSTATE(1); }
        else if (MATCH_TRACK_THRS('n','o','t', 3)) { NEWSTATE(3); }
       }
      else if (trial=='6') // N
       {
        j=scanpos; i=0;
        if ((in[j]=='+')||(in[j]=='-')) j++;
        while (isdigit(in[j])) { j++; i=1; }
        if (in[j]=='.') { j++; while (isdigit(in[j])) { j++; i=1; } }
        if (i==0) continue;
        if ((in[j]=='e')||(in[j]=='E'))
         {
          j++;
          if ((in[j]=='+')||(in[j]=='-')) j++;
          while (isdigit(in[j])) j++;
         }
        j-=scanpos; NEWSTATE(j);
       }
      else if (trial=='7') // O
       {
        if (MATCH_TRACK_THRS('a','n','d',13))
         { NEWSTATE(3); }
        else if (MATCH_TRACK_TWO('*','*', 4)||MATCH_TRACK_TWO('<','<', 7)||MATCH_TRACK_TWO('>','>', 7)||MATCH_TRACK_TWO('<','=', 8)||MATCH_TRACK_TWO('>','=', 8)||
                 MATCH_TRACK_TWO('=','=', 9)||MATCH_TRACK_TWO('!','=', 9)||MATCH_TRACK_TWO('o','r',14)||MATCH_TRACK_TWO('<','>', 9))
         { NEWSTATE(2); }
        else if (MATCH_TRACK_ONE('+', 6)||MATCH_TRACK_ONE('-', 6)||MATCH_TRACK_ONE('*', 5)||MATCH_TRACK_ONE('/', 5)||MATCH_TRACK_ONE('%', 5)||MATCH_TRACK_ONE('&',10)||
                 MATCH_TRACK_ONE('|',12)||MATCH_TRACK_ONE('^',11)||MATCH_TRACK_ONE('<', 8)||MATCH_TRACK_ONE('>', 8))
         { NEWSTATE(1); }
       }
      else if ((trial=='8') && (isalpha(in[scanpos]))) // V
       {
        NEWSTATE(1);
        while ((isalnum(in[scanpos])) || (in[scanpos]=='_')) { SAMESTATE; }
       }
      if (state != oldstate) break;
     }
    if (state == oldstate) break; // We've got stuck
   }
  if (state == '2') // We reached state 2(E)... end of expression
   {
    *errpos = -1; *errtext='\0';
    status[scanpos]=0;
    *end = scanpos;
    //for (i=0;i<scanpos;i++) printf("%c",in[i]); printf("\n"); // DEBUG LINES
    //for (i=0;i<scanpos;i++) printf("%c",(status[i]+'0')); printf("\n");
    //for (i=0;i<OPLIST_LEN;i++) printf("%c",(OpList[i]+'0')); printf("\n");
    return;
   }
  *errpos = scanpos; // Error; we didn't reach state 2(E)
  *end    = -1;
  // Now we need to construct an error string
  strcpy(errtext,"Syntax Error: At this point, was expecting "); i=strlen(errtext); j=0;
  for (trialpos=0; ((trial=AllowedNext[(int)(state-'1')][trialpos])!='\0'); trialpos++ )
   {
    if ((trial=='4') && (DollarAllowed == 0)) continue;
    if (j!=0) {strcpy(errtext+i," or "); i+=strlen(errtext+i);} else j=1;
    if      (trial=='3') { strcpy(errtext+i,"a bracketed expression"); i+=strlen(errtext+i); }
    else if (trial=='4') { strcpy(errtext+i,"a dollar"); i+=strlen(errtext+i); }
    else if (trial=='5') { strcpy(errtext+i,"a minus sign"); i+=strlen(errtext+i); }
    else if (trial=='6') { strcpy(errtext+i,"a numeric value"); i+=strlen(errtext+i); }
    else if (trial=='7') { strcpy(errtext+i,"an operator"); i+=strlen(errtext+i); }
    else if (trial=='8') { strcpy(errtext+i,"a variable or function name"); i+=strlen(errtext+i); }
   }
  strcpy(errtext+i,"."); i+=strlen(errtext+i);
  return;
 }

