/*
 *  cpu_linux.c - get cpu usage
 *
 *  Copyright (C) 2003 Draghicioiu Mihai <misuceldestept@go.ro>
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
 *  Foundation, Inc., 59 Temple Street #330, Boston, MA 02111-1307, USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

long cpu_used;
long cpu_total;
int  cpu_num;
long oldused;
long oldtotal;

void cpu_init(int c)
{
 cpu_num = c;
}

void cpu_getusage()
{
 FILE *file;
 long cpu, nice, system, idle, used, total;
 char buf[128];
 int c = -1;

 file = fopen("/proc/stat", "r");
 if(!file)
 {
  perror("/proc/stat");
  exit(1);
 }
 while(c != cpu_num)
 {
  if(fgets(buf, 128, file) == NULL)
  {
   fprintf(stderr, "Cannot stat cpu.\n");
   exit(1);
  }
  sscanf(buf, "%*03s%d %ld %ld %ld %ld", &c, &cpu, &nice, &system, &idle);
 }
 fclose(file);
 used =  cpu + nice + system;
 total = used + idle;
 cpu_used = used - oldused;
 cpu_total = total - oldtotal;
 oldused = used;
 oldtotal = total;
}
