/*
  Module       : utils.c
  Purpose      : Various utilities for scansreader
  More         : see scansreader README
  Policy       : GNU GPL
  Homepage     : http://www.klografx.net/scansreader/
*/	

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <gdk/gdkx.h>
#include <dirent.h>
#include <ctype.h>
#include <regex.h>
#include "scansreader.h"

#ifdef STAT_MACROS_BROKEN
#undef S_ISDIR
#endif

#if !defined(S_ISDIR) && defined(S_IFDIR)
#define S_ISDIR(m) (((m) & S_IFMT) == S_IFDIR)
#endif
  
/* 
   This routine jumps x images forward or backward or
   directly to image x
   Enter jf10\n ... jumps 10 images forward
   Enter jb5\n  ... jumps 5 images backward
   Enter jt15\n ... jumps to image 15
*/
void jump2image(char *cmd)
{
  int direction = 0;
  int x;

#ifdef DEBUG
    g_print("*** starting jump2image function: '%s'\n", cmd);
#endif

  if(cmd[0] == 'f' || cmd[0] == 'F')
    direction = 1;
  else if(cmd[0] == 'b' || cmd[0] == 'B')
    direction = -1;
  else if(!(cmd[0] == 't' || cmd[0] == 'T'))
    return;

  /* get number of images to jump or image to jump to */
  x = atoi(cmd+1);

  if (direction == 1) {
    if ((image_idx + x) > (images-1))
      image_idx = images-1;
    else
      image_idx += x;
  }
  else if (direction == -1) {
    if ((image_idx - x) < 0)
      image_idx = 0;
    else
      image_idx -= x;
  }
  else {
    if (x > images || x < 1)
      return;
    else
      image_idx = x-1;
  }

#ifdef DEBUG
    g_print("*** end of jump2image function\n");
#endif

}

void finish(int sig)
{
  gdk_pointer_ungrab(CurrentTime);
  gdk_keyboard_ungrab(CurrentTime);
  scansreader_exit(0);
} 

/*
  Update selected image index image_idx
  Direction determines if the next or the previous
  image is selected.
*/
void next_image(int direction)
{
  static int last_modif = 1;	/* Delta of last change of index of image */

  if (!direction)
    direction = last_modif;
  else
    last_modif = direction;
  if (random_order)
    image_idx = get_random(random_replace, images, direction);
  else
    image_idx = (image_idx + direction + images) % images;
}

/* returns a random number from the integers 0..num-1, either with
   replacement (replace=1) or without replacement (replace=0) */

int get_random(int replace, int num, int direction)
{
  static int index = -1;
  static int *rindices = NULL;  /* the array of random intgers */
  static int rsize;

  int n,m,p,q;

  if (!rindices)
    rindices = (int *) malloc((unsigned) max_rand_num*sizeof(int));
  if (rsize != num) {
    rsize = num;
    index = -1;
  }

  if (index < 0)         /* no more indices left in this cycle. Build a new */
    {		         /* array of random numbers, by not sorting on random keys */
      index = num-1;

      for (m=0;m<num;m++)
	{
	  rindices[m] = m; /* Make an array of growing numbers */
	}

      for (n=0;n<num;n++)   /* simple insertion sort, fine for num small */
	{
	  p=(int)(((float)rand()/RAND_MAX) * (num-n)) + n ; /* n <= p < num */
	  q=rindices[n];
	  rindices[n]=rindices[p]; /* Switch the two numbers to make random order */
	  rindices[p]=q;
	}
    }

  return rindices[index--];
}

/* Recursively gets all files from a directory */

int rreaddir(const char *dirname)
{
    DIR *d;
    struct dirent *entry;
    char cdirname[FILENAME_LEN], name[FILENAME_LEN];
    struct stat sb;
    int before_count = images;

    strncpy(cdirname, dirname, sizeof cdirname);
    cdirname[FILENAME_LEN-1] = '\0';

    if(!(d = opendir(cdirname)))
	return -1;
    while((entry = readdir(d)) != NULL) {
	if(!strcmp(entry->d_name,".") || !strcmp(entry->d_name,"..")) {
	    continue;
	}
	snprintf(name, sizeof name, "%s/%s", cdirname, entry->d_name);
	if (stat(name, &sb) >= 0 && S_ISDIR(sb.st_mode) && recursive) {
	    rreaddir(name);
	}
	else {
	    if (images >= max_image_cnt) {
		max_image_cnt += 8192;
		if (!image_names)
		    image_names = (char**)malloc(max_image_cnt * sizeof(char*));
		else
		    image_names = (char**)realloc(image_names,max_image_cnt*sizeof(char*));
	    }
	    image_names[images++] = strdup(name);
	}
    }
    closedir(d);
    return images - before_count;
}

gboolean color_alloc(const char *name, GdkColor *color)
{
    gboolean result;

    result = gdk_color_parse(name, color);

    if (!result) {
        fprintf(stderr, "scansreader: can't parse color '%s'\n", name);
        name = "black";
    }

    result = gdk_colormap_alloc_color(cmap, color, FALSE, TRUE);

    if (!result) {
        fprintf(stderr, "scansreader: can't alloc color '%s'\n", name);
        color->pixel = 0;
    }

    return result;
}

void swap(int *a, int *b)
{
    int temp;
    temp = *a;
    *a = *b;
    *b = temp;
}

/* rounding a float to an int */
int round( double a )
{
  return( (a-(int)a > 0.5) ? (int)a+1 : (int)a);
}

int keyval_main() {
  int keyval;
  if (images <= 1) return 0;
  keyval = ((1 + image_idx) * 9) / (images - 1);
  if (keyval++ == 9) keyval = 0; 
  return keyval;
}
int keyval_add() {
  int keyval;
  if (images <= 1) return 0;
  keyval = ((1 + image_idx) * 9) / (images - 1);
  return image_idx - (keyval * (images - 1)) / 9;
}

/* virtual chdir: if the arg is a .cbz or .cbr file, expands it in 
   /tmp/scansreader.$$/_hash_code_of_dirname
   and goto there. keeps the last dirname in vchdir_pwd
   return 0 on success
*/
char vchdir_pwd[8192];
void vchdir_set_pwd() { getcwd(vchdir_pwd, 8191); }
int vchdir(char *dirname) {
  char realdir[10000];
  char command[10000];
  char quoted_dirname[10000];

  strncpy(vchdir_pwd, dirname, 8191);
  shellquote(quoted_dirname, dirname);
  if (match("[.](cbz|bdz|zip|jar)$", dirname, 0)) {
    strcpy(realdir, scansreader_tmpdir());
    strcat(realdir, dir_hash_code(dirname));
    mkdir(realdir, 0777);
    sprintf(command, "unzip -o -qq -j '%s' -d '%s'", quoted_dirname, realdir);
    if (0 != system(command)) {
      fprintf(stderr, "scansreader: cannot expand '%s'\n", dirname);
      return 1;
    }
    return chdir(realdir);
  } else if (match("[.](cbr|bdr|rar)$", dirname, 0)) {
    strcpy(realdir, scansreader_tmpdir());
    strcat(realdir, dir_hash_code(dirname));
    mkdir(realdir, 0777);
    sprintf(command, "rar e -o+ '%s' '%s/'", quoted_dirname, realdir);
    if (0 != system(command)) {
      fprintf(stderr, "scansreader: cannot expand '%s'\n", dirname);
      return 1;
    }
    return chdir(realdir);
  } else {
    return chdir(dirname);
  }
}

/* 1 = success, 0 = failure */
int match(char *pattern, char *string, int obey_case)
{
   int status;
   regex_t re;

   if (regcomp(&re, pattern, 
	       (obey_case ? 0 : REG_ICASE) |REG_EXTENDED|REG_NOSUB) != 0)  {
     return(0) ;
   }
   status = regexec(&re, string, (size_t) 0, NULL, 0);
   regfree(&re);
   return (status ? 0 : 1);
}

char *scansreader_tmpdir() {
  static char tmpdir[32];
  if (tmpdir[0] == '\0') {
    sprintf(tmpdir, "/tmp/scansreader.%d/", getpid());
    mkdir(tmpdir, 0777);
  }
  return tmpdir;
}

void scansreader_tmpdir_clean() {
  char command[10000];
  sprintf(command, "rm -rf %s", scansreader_tmpdir());
  system(command);
}

#define HCSIZE 1000
char *dir_hash_code(char *dirname) {
  static char hashcode[HCSIZE];
  char *p, *q, *q_end;
  p=dirname, q=hashcode, q_end=hashcode+HCSIZE-1;
  while (*p) {
    if (q == q_end) *q = '\0', q=hashcode;
    if (*p == '/') *q++ = '_';
    else if (*p == '\'') *q++ = '"'; 
    else *q++ = *p;
    p++;
  }
  *q++ = '_'; *q = '\0';
  return hashcode;
}

void shellquote(char *dest, char *src) {
  char *p, *q;
  p = src, q = dest;
  while (*p) {
    if (*p == '\'') strcpy(q, "'\"'\"'"), q += 5;
    else *q++ = *p;
    p++;
  }
}
