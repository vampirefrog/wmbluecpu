/*
 *  WMBlueCPU - a cpu monitor
 *
 *  Copyright (C) 2003 Draghicioiu Mihai Andrei <misuceldestept@go.ro>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/select.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/xpm.h>
#include <X11/extensions/shape.h>

#include "cpu.h"
#include "mwm.h"
#include "menu.h"

#include "pixmap.xpm"
#include "icon.xpm"

#define WINDOW_WIDTH 64
#define WINDOW_HEIGHT 64
#define WINDOW_NAME "WMBlueCPU"
#define ICON_NAME "wmbluecpu"
#define CLASS_NAME "wmbluecpu"
#define CLASS_CLASS "WMBlueCPU"
#define GRAPH_HEIGHT 12
#define GRAPH_LENGTH 18
#define GRAPH_X 6
#define GRAPH_Y 24
#define GRAPH_PT_W 3
#define GRAPH_PT_H 3
#define PERCENT_X 28
#define PERCENT_Y 16
#define CPUNUM_X 13
#define CPUNUM_Y 16

#define OPT_DISPLAY NULL
#define OPT_MILISECS 1000
#define OPT_BGCOLOR  "rgb:00/00/00"
#define OPT_ONCOLOR  "rgb:87/D7/FF"
#define OPT_OFFCOLOR "rgb:00/95/E0"
#define OPT_WINDOW 0
#define OPT_SHAPE 1
#define OPT_CPUNUM 0

int            argc;
char         **argv;
Display       *display;
int            screen;
Colormap       colormap;
Visual        *visual;
int            depth;
GC             backgc, offgc, ongc;
Window         rootwindow, window, iconwindow, mapwindow;
XEvent         event;
int            exitloop;
Atom           wm_delete_window;
Atom           _motif_wm_hints;
MWMHints       mwmhints;
Pixmap         buffer, pixmap, pixmask, iconpixmap;
unsigned long  bgcolor, offcolor, oncolor;
struct timeval tv = { 0, 0 };
int            oldx, oldy;
menu_t *       m;


char *opt_display  = OPT_DISPLAY;
int   opt_milisecs = OPT_MILISECS;
char *opt_bgcolor  = OPT_BGCOLOR;
char *opt_offcolor = OPT_OFFCOLOR;
char *opt_oncolor  = OPT_ONCOLOR;
int   opt_window   = OPT_WINDOW;
int   opt_shape    = OPT_SHAPE;
int   opt_cpunum   = OPT_CPUNUM;

unsigned long get_color(char *colorname)
{
 XColor c, c2;
 c2.pixel = 0;
 XAllocNamedColor(display, colormap, colorname, &c, &c2);
 return c.pixel;
}

void bad_option(int arg)
{
 fprintf(stderr, "%s needs an argument.\n"
                 "Try `%s --help' for more information.\n",
                 argv[arg-1], argv[0]);
 exit(1);
}

void print_usage()
{
 printf("Usage: %s [option] [-c <cpunumber>]\n"
	"Options:\n"
	"    -h,  --help           Print out this help and exit\n"
	"    -v,  --version        Print out version number and exit\n"
	"    -d,  --display  <dpy> The X11 display to connect to\n"
	"    -m,  --milisecs <ms>  The number of milisecs between updates\n"
	"    -b,  --bgcolor  <col> The background color\n"
	"    -f,  --offcolor <col> Color for Off leds\n"
	"    -o,  --oncolor  <col> Color for On leds\n"
	"    -w,  --window         Run in a window\n"
	"    -nw, --no-window      Run as a dockapp\n"
	"    -s,  --shape          Use XShape extension\n"
	"    -ns, --no-shape       Don't use XShape extension\n"
	"    -c,  --cpunum   <cpu> The cpu to monitor (for SMP systems)\n",
	argv[0]);
 exit(0);
}

void print_version()
{
 printf(WINDOW_NAME " version " VERSION "\n");
 exit(0);
}

void parse_args()
{
 int n;

 for(n = 1; n < argc; n++)
 {
  if(!strcmp(argv[n], "-h") ||
     !strcmp(argv[n], "--help"))
  {
   print_usage();
  }
  else if(!strcmp(argv[n], "-v") ||
          !strcmp(argv[n], "--version"))
  {
   print_version();
  }
  else if(!strcmp(argv[n], "-d") ||
     !strcmp(argv[n], "--display"))
  {
   if(argc <= ++n)
   {
    bad_option(n);
   }
   opt_display = argv[n];
  }
  else if(!strcmp(argv[n], "-m") ||
          !strcmp(argv[n], "--milisecs"))
  {
   if(argc <= ++n)
   {
    bad_option(n);
   }
   opt_milisecs = strtol(argv[n], NULL, 0);
  }
  else if(!strcmp(argv[n], "-b") ||
          !strcmp(argv[n], "--bgcolor"))
  {
   if(argc <= ++n)
   {
    bad_option(n);
   }
   opt_bgcolor = argv[n];
  }
  else if(!strcmp(argv[n], "-f") ||
          !strcmp(argv[n], "--offcolor"))
  {
   if(argc <= ++n)
   {
    bad_option(n);
   }
   opt_offcolor = argv[n];
  }
  else if(!strcmp(argv[n], "-o") ||
          !strcmp(argv[n], "--oncolor"))
  {
   if(argc <= ++n)
   {
    bad_option(n);
   }
   opt_oncolor = argv[n];
  }
  else if(!strcmp(argv[n], "-w") ||
          !strcmp(argv[n], "--window"))
  {
   opt_window = 1;
  }
  else if(!strcmp(argv[n], "-nw") ||
          !strcmp(argv[n], "--no-window"))
  {
   opt_window = 0;
  }
  else if(!strcmp(argv[n], "-s") ||
          !strcmp(argv[n], "--shape"))
  {
   opt_shape = 1;
  }
  else if(!strcmp(argv[n], "-ns") ||
          !strcmp(argv[n], "--no-shape"))
  {
   opt_shape = 0;
  }
  else if(!strcmp(argv[n], "-c") ||
          !strcmp(argv[n], "--cpu"))
  {
   if(argc <= ++n)
   {
    bad_option(n);
   }
   opt_cpunum = strtol(argv[n], NULL, 0);
  }
  else
  {
   fprintf(stderr, "Bad option. Try `%s --help' for more information.\n", argv[0]);
   exit(1);
  }
 }
}

#include "common.c"

int old_cpu_graph = 0;
void draw_window()
{
 int cpu_graph;

 XDrawRectangle(display, buffer, offgc, 19, 5, 39, 10);
 XFillRectangle(display, buffer, backgc, 20, 6, 38, 9);
 if(cpu_total > 0)
  cpu_graph = cpu_used * 40 / cpu_total;
 else cpu_graph = 0;
 if(cpu_graph > 0)
  XDrawRectangle(display, buffer, ongc, 19, 5, cpu_graph - 1, 10);
 if(cpu_graph > 2)
  XDrawRectangle(display, buffer, ongc, 20, 6, cpu_graph - 3, 8);
 if(cpu_graph > 8)
  XFillRectangle(display, buffer, offgc, 23, 9, cpu_graph - 8, 3);
 XCopyArea(display, buffer, buffer, backgc,
           19, 18, 40, 38, 19, 21);
 XFillRectangle(display, buffer, offgc, 19, 18, 40, 2);
 if(old_cpu_graph > 0)
  XFillRectangle(display, buffer, ongc, 19, 18, old_cpu_graph, 2);
 old_cpu_graph = cpu_graph;
}

void proc()
{
 int i;
 fd_set fs;
 int fd = ConnectionNumber(display);

 process_events();
 FD_ZERO(&fs); FD_SET(fd, &fs);
 i = select(fd + 1, &fs, 0, 0, &tv);
 if(i == -1)
 {
  fprintf(stderr, "Error with select(): %s", strerror(errno));
  exit(1);
 }
 if(!i)
 {
  cpu_getusage();
  draw_window();
  update_window();
  tv.tv_sec = opt_milisecs / 1000;
  tv.tv_usec = (opt_milisecs % 1000) * 1000;
 }
}

void free_stuff()
{
 XUnmapWindow(display, mapwindow);
 XDestroyWindow(display, iconwindow);
 XDestroyWindow(display, window);
 XFreePixmap(display, buffer);
 XFreePixmap(display, iconpixmap);
 XFreePixmap(display, pixmask);
 XFreePixmap(display, pixmap);
 XFreeGC(display, ongc);
 XFreeGC(display, offgc);
 XFreeGC(display, backgc);
 XCloseDisplay(display);
}

void set_refresh(menuitem_t *i)
{
 opt_milisecs = i->i;
 
 for(i = m->first; i && i->i > 0; i = i->next)
  if(i->i == opt_milisecs) i->checked = 1;
  else i->checked = 0;

 tv.tv_sec = 0;
 tv.tv_usec = 0;
}

int main(int ac, char **av)
{
 menuitem_t *i;

 argc = ac;
 argv = av;
 parse_args();
 make_window();
 menu_init(display);
 m = menu_new();
 i = menu_append(m, "Refresh: 250ms");
 i->i = 250; i->callback = set_refresh; if(opt_milisecs == 250) i->checked = 1; else i->checked = 0;
 i = menu_append(m, "Refresh: 500ms");
 i->i = 500; i->callback = set_refresh; if(opt_milisecs == 500) i->checked = 1; else i->checked = 0;
 i = menu_append(m, "Refresh: 1s");
 i->i = 1000; i->callback = set_refresh; if(opt_milisecs == 1000) i->checked = 1; else i->checked = 0;
 i = menu_append(m, "Refresh: 2s");
 i->i = 2000; i->callback = set_refresh; if(opt_milisecs == 2000) i->checked = 1; else i->checked = 0;
 i = menu_append(m, "Refresh: 4s");
 i->i = 4000; i->callback = set_refresh; if(opt_milisecs == 4000) i->checked = 1; else i->checked = 0;
 menu_append(m, "Exit");
 cpu_init(opt_cpunum);
 while(!exitloop)
  proc();
 free_stuff();
 return 0;
}
