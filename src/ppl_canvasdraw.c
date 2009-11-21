// ppl_canvasdraw.c
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

#define _PPL_CANVASDRAW_C 1

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <unistd.h>
#include <wordexp.h>

#include "EPSMaker/eps_comm.h"
#include "EPSMaker/eps_arrow.h"
#include "EPSMaker/eps_eps.h"
#include "EPSMaker/eps_image.h"
#include "EPSMaker/eps_plot.h"
#include "EPSMaker/eps_settings.h"
#include "EPSMaker/eps_text.h"

#include "StringTools/asciidouble.h"

#include "ppl_canvasitems.h"
#include "ppl_canvasdraw.h"
#include "ppl_children.h"
#include "ppl_datafile.h"
#include "ppl_error.h"
#include "ppl_settings.h"
#include "ppl_setting_types.h"
#include "pyxplot.h"

// Handy routine for copying files
static int filecopy(const char *in, const char *out)
 {
  int status=0, n;
  FILE *fi, *fo;
  DataFile_CreateBackupIfRequired(out);
  fi = fopen(in, "rb");
  fo = fopen(out, "wb");
  if ((fo==NULL)||(fi==NULL)) status=1;
  while ((!status) && ((n=fread((void *)temp_err_string, 1, FNAME_LENGTH, fi)) > 0))
   if (fwrite((void *)temp_err_string, 1, n, fo) < n)
    status=1;
  if (fi != NULL) fclose(fi);
  if (fo != NULL) fclose(fo);
  return status;
 }

// Run ghostscript to convert postscript output into bitmap graphics
#define BITMAP_TERMINAL_CLEANUP(X, Y) \
    if (system(temp_err_string) != 0) /* Run ghostscript */ \
     { ppl_error(ERR_GENERAL, "Error encountered whilst using " X " to generate " Y " output"); } \
    else \
     { \
      if (filecopy(GSOutputTemp, comm.FinalFilename) != 0) /* Move ghostscript output to desire target file */ \
       { sprintf(temp_err_string, "Could not write output to file '%s'", comm.FinalFilename); ppl_error(ERR_FILE, temp_err_string); } \
      remove(GSOutputTemp);  /* Delete temporary files */ \
     } \
    remove(comm.EPSFilename);

// Standard postscript prolog text for EPS and PS files
static char *EPS_PROLOG_TEXT = "\
";
static char *PS_PROLOG_TEXT = "\
/BeginEPSF { %def                                     % Prepare for EPS file\n\
 /b4_Inc_state save def                               % Save state for cleanup\n\
 /dict_count countdictstack def                       % Count objects on dict stack\n\
 /op_count count 1 sub def                            % Count objects on operand stack\n\
 userdict begin                                       % Push userdict on dict stack\n\
 /showpage { } def                                    % Redefine showpage, { } = null proc\n\
 0 setgray 0 setlinecap 1 setlinewidth 0 setlinejoin  % Prepare graphics state\n\
 10 setmiterlimit [ ] 0 setdash newpath\n\
 /languagelevel where                                 % If level not equal to 1 then set strokeadjust and overprint to their defaults.\n\
  {pop languagelevel\n\
   1 ne\n\
    {false setstrokeadjust false setoverprint\n\
    } if\n\
  } if\n\
} bind def\n\
\n\
/EndEPSF { %def                                       % Clean up after including an EPS file\n\
 count op_count sub {pop} repeat                      % Clean up stacks\n\
 countdictstack dict_count sub {end} repeat\n\
 b4_Inc_state restore\n\
} bind def\n\
";

// Standard commandline flags to pass to ghostscript
static char *GHOSTSCRIPT_STANDARD_FLAGS = "-dQUIET -dSAFER -dBATCH -dNOPAUSE -dEPSCrop";

// Table of the functions we call for each phase of the canvas drawing process for different object types
static void(*ArrowHandlers[])(EPSComm *) = {NULL                       , NULL                     , NULL                        , NULL                    , NULL                , NULL                , eps_arrow_RenderEPS, NULL};
static void(*EPSHandlers[]  )(EPSComm *) = {NULL                       , NULL                     , NULL                        , NULL                    , NULL                , NULL                , eps_eps_RenderEPS  , NULL};
static void(*ImageHandlers[])(EPSComm *) = {NULL                       , NULL                     , NULL                        , NULL                    , NULL                , NULL                , eps_image_RenderEPS, NULL};
static void(*PlotHandlers[] )(EPSComm *) = {eps_plot_ReadAccessibleData, eps_plot_DecideAxisRanges, eps_plot_LinkedAxesPropagate, eps_plot_SampleFunctions, eps_plot_YieldUpText, NULL                , eps_plot_RenderEPS , NULL};
static void(*TextHandlers[] )(EPSComm *) = {NULL                       , NULL                     , NULL                        , NULL                    , eps_text_YieldUpText, NULL                , eps_text_RenderEPS , NULL};
static void(*AfterHandlers[])(EPSComm *) = {NULL                       , NULL                     , NULL                        , NULL                    , canvas_CallLaTeX    , canvas_MakeEPSBuffer, canvas_EPSWrite    , NULL};

// Main entry point for rendering a canvas to graphical output
void canvas_draw(unsigned char *unsuccessful_ops)
 {
  int i, j, termtype, status=0, CSPCommand;
  static long TempFile_counter=0;
  char EPSFilenameTemp[FNAME_LENGTH], TitleTemp[FNAME_LENGTH], FinalFilenameTemp[FNAME_LENGTH], GSOutputTemp[FNAME_LENGTH];
  wordexp_t WordExp;
  char *EnvDisplay;
  EPSComm comm;
  canvas_item *item;
  void(*ArrowHandler)(EPSComm *);
  void(*EPSHandler  )(EPSComm *);
  void(*ImageHandler)(EPSComm *);
  void(*PlotHandler )(EPSComm *);
  void(*TextHandler )(EPSComm *);
  void(*AfterHandler)(EPSComm *);

  // By default, we record all operations as having been successful
  for (i=0;i<MULTIPLOT_MAXINDEX; i++) unsuccessful_ops[i]=0;

  // Work out filename to save output postscript to
  comm.termtype = termtype = settings_term_current.TermType;
  comm.FinalFilename = settings_term_current.output; // The final filename of whatever kind of output we're planning to produce
  if ((comm.FinalFilename==NULL)||(comm.FinalFilename[0]=='\0'))
   {
    comm.FinalFilename = FinalFilenameTemp; // If final target filename is blank, use pyxplot.<filetype>
    sprintf(FinalFilenameTemp, "pyxplot.%s", *(char **)FetchSettingName(termtype, SW_TERMTYPE_INT, (void *)SW_TERMTYPE_STR, sizeof(char *)));
   }

  // Perform expansion of shell filename shortcuts such as ~
  if ((wordexp(comm.FinalFilename, &WordExp, 0) != 0) || (WordExp.we_wordc <= 0)) { sprintf(temp_err_string, "Could not find directory containing filename '%s'.", comm.FinalFilename); ppl_error(ERR_FILE, temp_err_string); return; }
  if  (WordExp.we_wordc > 1) { sprintf(temp_err_string, "Filename '%s' is ambiguous.", comm.FinalFilename); ppl_error(ERR_FILE, temp_err_string); return; }
  strcpy(FinalFilenameTemp, WordExp.we_wordv[0]);
  wordfree(&WordExp);
  comm.FinalFilename = FinalFilenameTemp;

  // Case 1: EPS and PS terminals. Postscript output will be saved immediately in situ.
  if ((termtype == SW_TERMTYPE_EPS) || (termtype == SW_TERMTYPE_PS))
   {
    comm.EPSFilename = comm.FinalFilename; // eps or ps is actually what we're aiming to make, so save straight to user's chosen target
    DataFile_CreateBackupIfRequired(comm.EPSFilename);
    comm.title    = comm.EPSFilename; // Filename 'foo/bar/myplot.eps' --> title 'myplot.eps'
    for (i=0; comm.title[i]!='\0'; i++) if ((comm.title[i]==PATHLINK[0])&&((i==0)||(comm.title[i-1]!='\\'))) { comm.title += i; i=0; }
   }
  else // Case 2: All other terminals. Create a temporary eps file for subsequent processing.
   {
    comm.EPSFilename = EPSFilenameTemp;
    TempFile_counter++;
    sprintf(EPSFilenameTemp, "%s%spyxplot_%d_%ld.eps", settings_session_default.tempdir, PATHLINK, getpid(), TempFile_counter);
    if ((termtype==SW_TERMTYPE_X11S) || (termtype==SW_TERMTYPE_X11M) || (termtype==SW_TERMTYPE_X11P))
     {
      comm.title    = TitleTemp; // In X11 terminals, we give eps files numbered titles
      sprintf(TitleTemp, "PyXPlot Output %ld", TempFile_counter);
     } else {
      comm.title = comm.FinalFilename; // In other terminals, we give eps files filename titles, with path cut off front
      for (i=0; comm.title[i]!='\0'; i++) if ((comm.title[i]==PATHLINK[0])&&((i==0)||(comm.title[i-1]!='\\'))) { comm.title += i; i=0; }
     }
   }

  // Make GSOutputTemp a temporary file that ghostscript can spit data out to (with a shell-safe filename)
  sprintf(GSOutputTemp, "%s%spyxplot_%d_%ld%s", settings_session_default.tempdir, PATHLINK, getpid(), TempFile_counter, (termtype==SW_TERMTYPE_GIF)?".gif":".tmp");

  // Set up communications data structure for objects we are rendering
  comm.itemlist    = canvas_items;
  comm.bb_left     = comm.bb_right = comm.bb_top = comm.bb_bottom = 0.0;
  comm.bb_set      = 0;
  comm.epsbuffer   = NULL;
  comm.status      = &status;
  comm.LastEPSColour[0] = '\0';
  comm.LastLinewidth    = -1.0;

  // Prepare a buffer into which strings to be passed to LaTeX will be put

  // Rendering of EPS occurs in a series of phases which we now loop over
  for (j=0 ; ; j++)
   {
    ArrowHandler = ArrowHandlers[j]; // Each object type has a handler for each phase of postscript generation
    EPSHandler   = EPSHandlers  [j];
    ImageHandler = ImageHandlers[j];
    PlotHandler  = PlotHandlers [j];
    TextHandler  = TextHandlers [j];
    AfterHandler = AfterHandlers[j];
    if ((ArrowHandler==NULL)&&(EPSHandler==NULL)&&(ImageHandler==NULL)&&(PlotHandler==NULL)&&(TextHandler==NULL)&&(AfterHandler==NULL)) break;

    // Loop over all of the items on the canvas
    for (item=comm.itemlist->first; item!=NULL; item=item->next)
     {
      if (unsuccessful_ops[item->id]) continue; // ... except those which have already failed
      comm.current = item;
      if      ((item->type == CANVAS_ARROW) && (ArrowHandler != NULL)) (*ArrowHandler)(&comm); // Call the relevant handler for each one
      else if ((item->type == CANVAS_EPS  ) && (EPSHandler   != NULL)) (*EPSHandler  )(&comm);
      else if ((item->type == CANVAS_IMAGE) && (ImageHandler != NULL)) (*ImageHandler)(&comm);
      else if ((item->type == CANVAS_PLOT ) && (PlotHandler  != NULL)) (*PlotHandler )(&comm);
      else if ((item->type == CANVAS_TEXT ) && (TextHandler  != NULL)) (*TextHandler )(&comm);
      if (status) { unsuccessful_ops[item->id] = 1; } // If something went wrong... flag it up and give up on this object
      status = 0;
     }
    if (AfterHandler != NULL) (*AfterHandler)(&comm); // At the end of each phase, a canvas-wide handler may be called
    if (status) { return; }
   }

  // Now convert eps output to bitmaped graphics if requested
  if ((termtype==SW_TERMTYPE_X11S) || (termtype==SW_TERMTYPE_X11M) || (termtype==SW_TERMTYPE_X11P)) // X11 terminals
   {
    EnvDisplay = getenv("DISPLAY"); // Check whether the environment variable DISPLAY is set
    if (strcmp(GHOSTVIEW_COMMAND, "/bin/false")==0)
     {
      ppl_error(ERR_GENERAL, "An attempt is being made to use an X11 terminal for output, but the required package 'ghostview' could not be found when PyXPlot was installed. If you have recently install ghostview, please reconfigure and recompile PyXPlot.");
     }
    else if ((EnvDisplay==NULL) || (EnvDisplay[0]=='\0'))
     {
      ppl_error(ERR_GENERAL, "An attempt is being made to use an X11 terminal for output, but your DISPLAY environment variable is not set; there is no accessible X11 display.");
     }
    else
     {
      if (!WillBeInteractive)
       {
        if (termtype!=SW_TERMTYPE_X11P)
         {
          sprintf(temp_err_string, "An attempt is being made to use the %s terminal in a non-interactive PyXPlot session. This won't work, as the window will close as soon as PyXPlot exits. Reverting to the X11_persist terminal instead.", *(char **)FetchSettingName(termtype, SW_TERMTYPE_INT, (void *)SW_TERMTYPE_STR, sizeof(char *)));
          ppl_error(ERR_GENERAL, temp_err_string);
         }
        CSPCommand = 2;
       } else {
        if      (termtype==SW_TERMTYPE_X11S) CSPCommand = 0;
        else if (termtype==SW_TERMTYPE_X11M) CSPCommand = 1;
        else if (termtype==SW_TERMTYPE_X11P) CSPCommand = 2;
       }
      sprintf(temp_err_string, "%d%s\n", CSPCommand, comm.EPSFilename);
      SendCommandToCSP(temp_err_string);
     }
   }
  else if (termtype==SW_TERMTYPE_PDF) // PDF terminal
   {
    sprintf(temp_err_string, "%s %s -sDEVICE=pdfwrite -sOutputFile=%s %s",
        GHOSTSCRIPT_COMMAND, GHOSTSCRIPT_STANDARD_FLAGS, GSOutputTemp, comm.EPSFilename);
    BITMAP_TERMINAL_CLEANUP("ghostscript", "pdf");
   }
  else if (termtype==SW_TERMTYPE_PNG) // PNG terminal
   {
    sprintf(temp_err_string, "%s %s -sDEVICE=%s -r%d %s -sOutputFile=%s %s",
        GHOSTSCRIPT_COMMAND, GHOSTSCRIPT_STANDARD_FLAGS,
        (settings_term_current.TermTransparent==SW_ONOFF_ON)?"pngalpha":"png16m",
        (int)floor(settings_term_current.dpi),
        (settings_term_current.TermAntiAlias==SW_ONOFF_ON)?"-dGraphicsAlphaBits=4 -dTextAlphaBits=4":"-dGraphicsAlphaBits=0 -dTextAlphaBits=0",
        GSOutputTemp, comm.EPSFilename);
    BITMAP_TERMINAL_CLEANUP("ghostscript", "png")
   }
  else if (termtype==SW_TERMTYPE_JPG) // JPEG terminal
   {
    sprintf(temp_err_string, "%s %s -sDEVICE=jpeg -dJPEGQ=100 -r%d %s -sOutputFile=%s %s",
        GHOSTSCRIPT_COMMAND, GHOSTSCRIPT_STANDARD_FLAGS,
        (int)floor(settings_term_current.dpi),
        (settings_term_current.TermAntiAlias==SW_ONOFF_ON)?"-dGraphicsAlphaBits=4 -dTextAlphaBits=4":"-dGraphicsAlphaBits=0 -dTextAlphaBits=0",
        GSOutputTemp, comm.EPSFilename);
    BITMAP_TERMINAL_CLEANUP("ghostscript", "jpeg")
   }
  else if (termtype==SW_TERMTYPE_GIF) // GIF terminal
   {
    sprintf(temp_err_string, "%s -density %d %s %s %s %s",
        CONVERT_COMMAND,
        (int)floor(settings_term_current.dpi),
        (settings_term_current.TermTransparent==SW_ONOFF_ON)?"-transparent white":"",
        (settings_term_current.TermAntiAlias==SW_ONOFF_ON)?"-antialias":"+antialias",
        comm.EPSFilename, GSOutputTemp);
    printf("%s\n",temp_err_string);
    BITMAP_TERMINAL_CLEANUP("ImageMagick", "gif");
   }
  else if (termtype==SW_TERMTYPE_BMP) // BMP terminal
   {
    sprintf(temp_err_string, "%s %s -sDEVICE=bmp16m -r%d %s -sOutputFile=%s %s",
        GHOSTSCRIPT_COMMAND, GHOSTSCRIPT_STANDARD_FLAGS,
        (int)floor(settings_term_current.dpi),
        (settings_term_current.TermAntiAlias==SW_ONOFF_ON)?"-dGraphicsAlphaBits=4 -dTextAlphaBits=4":"-dGraphicsAlphaBits=0 -dTextAlphaBits=0",
        GSOutputTemp, comm.EPSFilename);
    BITMAP_TERMINAL_CLEANUP("ghostscript", "bmp")
   }
  else if (termtype==SW_TERMTYPE_TIF) // TIF terminal
   {
    sprintf(temp_err_string, "%s %s -sDEVICE=tiff24nc -r%d %s -sOutputFile=%s %s",
        GHOSTSCRIPT_COMMAND, GHOSTSCRIPT_STANDARD_FLAGS,
        (int)floor(settings_term_current.dpi),
        (settings_term_current.TermAntiAlias==SW_ONOFF_ON)?"-dGraphicsAlphaBits=4 -dTextAlphaBits=4":"-dGraphicsAlphaBits=0 -dTextAlphaBits=0",
        GSOutputTemp, comm.EPSFilename);
    BITMAP_TERMINAL_CLEANUP("ghostscript", "tif")
   }
  else if (termtype==SW_TERMTYPE_SVG) // SVG terminal
   {
    sprintf(temp_err_string, "%s %s -sDEVICE=svg -sOutputFile=%s %s > /dev/null 2> /dev/null",
        GHOSTSCRIPT_COMMAND, GHOSTSCRIPT_STANDARD_FLAGS,
        GSOutputTemp, comm.EPSFilename);
    BITMAP_TERMINAL_CLEANUP("ghostscript", "svg")
   }

  // Return to user's current working directory
  if (chdir(settings_session_default.cwd) < 0) { ppl_fatal(__FILE__,__LINE__,"chdir into cwd failed."); }
  return;
 }

// Run LaTeX to turn set of strings for typesetting into eps code
void canvas_CallLaTeX(EPSComm *x)
 {
  // chdir into temporary directory so that LaTeX's mess goes into /tmp
  if (chdir(settings_session_default.tempdir) < 0) { ppl_error(ERR_INTERNAL,"Could not chdir into temporary directory."); *(x->status)=1; return; }

  // Return to user's current working directory after LaTeX has finished making a mess
  if (chdir(settings_session_default.cwd) < 0) { ppl_fatal(__FILE__,__LINE__,"chdir into cwd failed."); }
  return;
 }

// Make a temporary buffer file into which the eps fragments for objects is temporarily stored
void canvas_MakeEPSBuffer(EPSComm *x)
 {
  x->epsbuffer = tmpfile();
  if (x->epsbuffer == NULL) { ppl_error(ERR_INTERNAL,"Could not create temporary eps buffer file."); *(x->status)=1; return; }
  return;
 }

// Finally output postscript file, once all of the eps fragments are written to temporary buffer and bounding box is known
void canvas_EPSWrite(EPSComm *x)
 {
  FILE *epsout;
  char LandscapifyText[FNAME_LENGTH], EnlargementText[FNAME_LENGTH];

  // Apply enlarge and landscape terminals as required
  LandscapifyText[0] = EnlargementText[0] = '\0';
  if  (settings_term_current.landscape   == SW_ONOFF_ON)                                     canvas_EPSLandscapify(x, LandscapifyText);
  if ((settings_term_current.TermEnlarge == SW_ONOFF_ON) && (x->termtype == SW_TERMTYPE_PS)) canvas_EPSEnlarge    (x, EnlargementText);

  // Return to user's current working directory
  if (chdir(settings_session_default.cwd) < 0) { ppl_fatal(__FILE__,__LINE__,"chdir into cwd failed."); }

  // Open output postscript file for writing
  if ((epsout=fopen(x->EPSFilename,"w"))==NULL) { sprintf(temp_err_string, "Could not open file '%s' for writing.", x->EPSFilename); ppl_error(ERR_FILE, temp_err_string); *(x->status)=1; return; }

  // Write EPS header
  fprintf(epsout, "%s%s\n", "%!PS-Adobe-3.0", (settings_term_current.TermType == SW_TERMTYPE_PS)?"":" EPSF-3.0");
  fprintf(epsout, "%%%%BoundingBox %d %d %d %d\n", (int)floor(x->bb_left), (int)floor(x->bb_bottom), (int)ceil(x->bb_right), (int)ceil(x->bb_top));
  fprintf(epsout, "%%%%HiResBoundingBox: %f %f %f %f\n", x->bb_left, x->bb_bottom, x->bb_right, x->bb_top);
  fprintf(epsout, "%%%%Creator: (PyXPlot %s)\n", VERSION);
  fprintf(epsout, "%%%%Title: (%s)\n", x->title);
  fprintf(epsout, "%%%%CreationDate: (%s)\n", StrStrip(FriendlyTimestring(), temp_err_string));
  if (settings_term_current.TermType == SW_TERMTYPE_PS) fprintf(epsout, "%%%%Pages: 1\n");
  fprintf(epsout, "%%%%EndComments\n\n");

  // Write EPS prolog
  fprintf(epsout, "%%%%BeginProlog\n");
  if (settings_term_current.TermType == SW_TERMTYPE_PS) fprintf(epsout, "%s", PS_PROLOG_TEXT);
  fprintf(epsout, "%s", EPS_PROLOG_TEXT);
  fprintf(epsout, "%%%%EndProlog\n\n");

  // In postscript files, now set up page 1
  if (settings_term_current.TermType == SW_TERMTYPE_PS)
   {
    fprintf(epsout, "%%%%Page: 1 1\n");
    fprintf(epsout, "%%%%BeginPageSetup\n");
    fprintf(epsout, "/pgsave save def\n");
    fprintf(epsout, "%%%%EndPageSetup\n\n");
   }

  // Now write any global transformations needed by the enlarge and landscape terminals.
  if ((LandscapifyText[0]!='\0')||(EnlargementText[0]!='\0'))
   {
    fprintf(epsout, "%% Global transformations to give %soutput%s", (LandscapifyText[0]!='\0')?"landscape ":"", (EnlargementText[0]!='\0')?" which fills tha page":"");
    fprintf(epsout, "%s", LandscapifyText);
    fprintf(epsout, "%s", EnlargementText);
    fprintf(epsout, "\n");
   }

  // Copy contents of eps buffer into postscript output
  fflush(x->epsbuffer);
  rewind(x->epsbuffer);
  while (fgets(temp_err_string, FNAME_LENGTH, x->epsbuffer) != NULL)
   if (fputs(temp_err_string, epsout) == EOF)
    {
     sprintf(temp_err_string, "Error while writing to file '%s'.", x->EPSFilename); ppl_error(ERR_FILE, temp_err_string);
     *(x->status)=1;
     return;
    }

  // Write postscript footer and finish
  if (settings_term_current.TermType == SW_TERMTYPE_PS) fprintf(epsout, "pgsave restore\n"); // End of page
  fprintf(epsout, "showpage\n%%%%EOF\n"); // End of document
  fclose(x->epsbuffer);
  fclose(epsout);
  return;
 }

// Convert a portrait EPS file to landscape by changing its bounding box and adding a linear transformation to the top
void canvas_EPSLandscapify(EPSComm *x, char *transform)
 {
  double width, height;
  width  = x->bb_right - x->bb_left;
  height = x->bb_top   - x->bb_bottom;
  sprintf(transform, "-90 rotate\n%f %f translate\n", -x->bb_right, -x->bb_bottom);
  x->bb_left   = 0.0;
  x->bb_bottom = 0.0;
  x->bb_right  = height;
  x->bb_top    = width;
  return;
 }

// Enlarge an EPS file to fill a page by changing its bounding box and adding a linear transformation to the top
void canvas_EPSEnlarge(EPSComm *x, char *transform)
 {
  double EPSwidth, EPSheight, PAGEwidth, PAGEheight;
  double margin_left, margin_right, margin_top, margin_bottom;
  double scaling_x, scaling_y, translate_x, translate_y;

  // Read dimensions of page
  PAGEwidth  = settings_term_current.PaperWidth .real;
  PAGEheight = settings_term_current.PaperHeight.real;

  // Calculate dimensions of EPS image
  EPSwidth  = x->bb_right - x->bb_left;
  EPSheight = x->bb_top   - x->bb_bottom;

  // Work out some sensible margins for page
  margin_left = PAGEwidth / 10;
  margin_top  = PAGEheight / 10;
  if (margin_left > 15e-3) margin_left = 15e-3; // mm
  if (margin_top  > 15e-3) margin_top  = 15e-3; // mm
  margin_right  =   margin_left;
  margin_bottom = 2*margin_top;

  // Work out what scaling factors will make page fit
  scaling_x = (PAGEwidth  - margin_left - margin_right ) / EPSwidth  * M_TO_PS;
  scaling_y = (PAGEheight - margin_top  - margin_bottom) / EPSheight * M_TO_PS;
  if (scaling_y < scaling_x) scaling_x = scaling_y; // Lowest scaling factor is biggest enlargement we can do

  // Work out how to translate origin to (0,0)
  translate_x =                          margin_left * M_TO_PS  -  x->bb_left * scaling_x;
  translate_y = PAGEheight * M_TO_PS  -  margin_top  * M_TO_PS  -  x->bb_top  * scaling_x;

  // Write out transformation
  sprintf(transform, "%f %f translate\n%f %f scale\n", translate_x, translate_y, scaling_x, scaling_x);
  x->bb_left   = margin_left * M_TO_PS;
  x->bb_right  = margin_left * M_TO_PS  +  EPSwidth * scaling_x;
  x->bb_top    = (PAGEheight - margin_top) * M_TO_PS;
  x->bb_bottom = (PAGEheight - margin_top) * M_TO_PS  -  EPSheight * scaling_x;
  return;
 }

