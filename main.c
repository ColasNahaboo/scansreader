/*
  Module       : main.c
  Purpose      : GDK/Imlib Quick Image Viewer (scansreader)
  More         : see scansreader README
  Homepage     : http://www.klografx.net/scansreader/
  Policy       : GNU GPL
*/	

#include <errno.h>
#include <gdk/gdkx.h>
#include <gtk/gtkwidget.h>
#include <stdio.h>
#include <signal.h>
#include <sys/time.h>
#include <ctype.h>
#include <string.h>
#include "scansreader.h"
#include "main.h"

scansreader_image main_img;

GdkImlibImage *next_im = NULL;			/* prefetched next image */
int next_image_idx = -1;

static void filter_images(int *, char **);
static int check_extension(const char *);
static void scansreader_signal_usr1();
static void scansreader_signal_usr2();
static gboolean scansreader_handle_timer(gpointer);
static void scansreader_timer_restart(gpointer);
void save_last_pos();
void do_the_right_thing (scansreader_image *);
static int initial_image_idx = 0;
extern int rreaddir(const char *);

int main(int argc, char **argv)
{
  struct timeval tv;

  //er, we don't set a value for params.dither here?
  //i would probably just let the user's .imrc set it
  //GdkImlibInitParams params;
  //params.flags = PARAMS_DITHER;

  /* Initialize GDK and Imlib */

  gdk_init(&argc,&argv);
  gdk_imlib_init();

  //probably should only use one of the init funcs...
  //gdk_imlib_init_params(&params);

  /* Load things from GDK/Imlib */

  scansreader_main_loop = g_main_new(TRUE);
  cmap = gdk_colormap_get_system();
  text_font = gdk_font_load(STATUSBAR_FONT);
  screen_x = gdk_screen_width();
  screen_y = gdk_screen_height();
  cursor = visible_cursor = gdk_cursor_new(CURSOR);

  gtk_widget_push_visual(gdk_imlib_get_visual());
  gtk_widget_push_colormap(gdk_imlib_get_colormap());

  /* Randomize seed for 'true' random */

  gettimeofday(&tv,NULL);
  srand(tv.tv_usec*1000000+tv.tv_sec);

  /* Set up our options, image list, etc */

  reset_mod(&main_img);
  helpstrs = helpstrs_us; helpkeys = helpstrs + 2;
  options_read(argc, argv, &main_img);
  max_rand_num = images;

  if (!strcmp(language, "fr")) {
    helpstrs = helpstrs_fr; helpkeys = helpstrs + 2;
  }

  if (filter) /* Filter graphic images */
    filter_images(&images,image_names);
  
  if (!images) { /* No images to display */
    g_print("scansreader: cannot load any images.\n");
    usage(argv[0],1);
  }

  /* get colors */

  color_alloc(STATUSBAR_BG, &text_bg);
  color_alloc(ERROR_BG, &error_bg);
  color_alloc(image_bg_spec, &image_bg);

  /* Display first image first, except in random mode */

  image_idx = 0;
  if (random_order) next_image(0);
  if (initial_image_idx) next_image(initial_image_idx);

  /* Setup callbacks */

  gdk_event_handler_set(scansreader_handle_event, &main_img, NULL);
  scansreader_timer_restart(NULL);

  /* And signal catchers */

  signal(SIGTERM, finish);
  signal(SIGINT, finish);
  signal(SIGUSR1, scansreader_signal_usr1);
  signal(SIGUSR2, scansreader_signal_usr2);

  /* Load & display the first image */

  scansreader_load_image(&main_img);
  g_main_run(scansreader_main_loop); /* will never return */
  return 0;
}


void scansreader_g_exit(int code, int save_position)
{
  if (save_position) save_last_pos();
  if (text_font) gdk_font_unref(text_font);
  if (cmap) gdk_colormap_unref(cmap);
  destroy_image(&main_img);

  g_main_destroy(scansreader_main_loop);
  finish(SIGTERM);		/* calls scansreader_exit(0) */
}

void scansreader_exit(int code)
{
  scansreader_tmpdir_clean();
  gdk_exit(code);
}


/*
 * functions for handling signals
 */

static void scansreader_signal_usr1()
{
  next_image(1);
  scansreader_load_image(&main_img);
}

static void scansreader_signal_usr2()
{
  next_image(-1);
  scansreader_load_image(&main_img);
}


/*
 * Slideshow timer function
 *
 * If this function returns false, the timer is destroyed
 * and scansreader_timer_restart() is automatically called, which
 * then starts the timer again. Thus images which takes some
 * time to load will still be displayed for "delay" seconds.
 */
 
static gboolean scansreader_handle_timer(gpointer data)
{
  if (*(int *)data || slide) {
    do_the_right_thing(&main_img);
      /*    next_image(0); */
  }
  return FALSE;
}


/*
 *	Slideshow timer (re)start function
 */
 
static void scansreader_timer_restart(gpointer dummy)
{
  g_timeout_add_full(G_PRIORITY_LOW, delay,
		     scansreader_handle_timer, &slide,
		     scansreader_timer_restart);
}

/* Filter images by extension */

static void filter_images(int *images, char **image_names)
{
  int i = 0;

  while(i < *images) {
    if (check_extension(image_names[i])) {
      i++;
    } else {
      int j = i;
      while(j < *images-1) {
	image_names[j] = image_names[j+1];
	++j;
      }
      --(*images);
    }
  }
}

static int check_extension(const char *name)
{
  char *extn = strrchr(name, '.');
  int i;

  if (extn)
    for (i=0; image_extensions[i]; i++)
      if (strcasecmp(extn, image_extensions[i]) == 0)
        return 1;

  return 0;
}

void remove_nl(char *s) {
  char *p = strrchr(s, '\n');if (p) *p = '\0';
}

void
save_last_pos() {
  FILE *fp;
  char savefile[1024];

  sprintf(savefile, "%s/.scansreader.log", getenv("HOME"));
  fp = fopen(savefile, "a");
  fprintf(fp, "%s\t%s\t%d\n", vchdir_pwd, image_names[image_idx], image_idx);
  fclose(fp);
}

static struct _place {
  int idx; char dir[1024]; char file[1024];
} *places;

void previous_place(int step) {
  static int cur_place;
  static int last_place;
  int new_place;
  
  if (places == NULL) {
    FILE *fp;
    char *homedir;
    char savefile[1024];
    char buffer[1024];
    int nmatch;
    homedir = getenv("HOME");
    sprintf(savefile, "%s/.scansreader.log", homedir);
    if ((fp = fopen(savefile, "r"))) {
      while (fgets(buffer, 1023, fp)) {
	places = realloc(places, (last_place + 1) * sizeof(struct _place));
	nmatch = sscanf(buffer, "%[^\t]\t%[^\t]\t%d\n", 
		  places[last_place].dir, places[last_place].file, 
			&(places[last_place].idx));
	if (nmatch != 3) {
	  g_print("scansreader: Warning: bad log line #%d: %s\n", last_place+1, 
		  buffer);
	  continue;
	}
	remove_nl(places[last_place].dir);
	remove_nl(places[last_place].file);
	cur_place = last_place;
	last_place++;
      }
    } else {
      cur_place = 0; last_place = 0;
    }
  }
  new_place = cur_place + step;
  if (new_place >= 0 && new_place < last_place) {
    cur_place = new_place;
    image_idx = initial_image_idx = places[new_place].idx;
    g_print("scansreader: resuming page %d in dir %s\n", 
	    initial_image_idx, places[new_place].dir);
    if (!vchdir(places[new_place].dir)) {
      if (image_names) {
	free(image_names);
	image_names = 0;
      }
      images = 0; max_image_cnt = 0;
      rreaddir(".");
    } else {
      g_print("scansreader: ***ERROR[%d]*** Could not go in dir: %s\n", 
	      errno, places[new_place].dir);
    }
  }
}

void
restore_last_pos() {
  previous_place(0);
}
