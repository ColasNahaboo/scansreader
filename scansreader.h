#include <gdk/gdk.h>
#include <gdk_imlib.h>
#include <unistd.h>
#include <stdlib.h>

#define VERSION "1.13"
#define URL "http://colas.nahaboo.net/scansreader/"
#define TRASH_DIR ".scansreader-trash"
#define SLIDE_DELAY 3000 /* milliseconds */
#define IMAGE_BG "black"
#define STATUSBAR_BG "#FFB900"
#define ERROR_BG "#0000FF"
#define DEFAULT_BRIGHTNESS 256
#define DEFAULT_CONTRAST 256
#define DEFAULT_GAMMA 256
#define BUF_LEN 1024

#define SCROLL_FACTOR 2 /* how many steps to move image? on space */
#define SCROLL_FACTOR_FINE 5 /* how many steps to move image? */
#define ZOOM_FACTOR 50 /* percent more at each step */

/* FILENAME_LEN is the maximum length of any path/filename that can be
 * handled.  MAX_DELETE determines how many items can be placed into
 * the kill-ring for undelete handling. */

#define FILENAME_LEN 1024
#define MAX_DELETE 1024

typedef struct _scansreader_image {
  GdkImlibColorModifier mod; /* Image modifier (for brightness..) */
  GdkPixmap *p; /* Pixmap of the image to display */
  GdkImlibImage *im; /* Image */
  GdkWindow *win; /* Main window for windowed and fullscreen mode */
  int error; /* 1 if Imlib couldn't load image */
  gint win_x, win_y, win_w, win_h; /* window co-ordinates */
  gint orig_w, orig_h; /* Size of original image in pixels */
  GdkGC *black_gc; /* Background GC (black), also for statusbar font */
  GdkGC *status_gc; /* Background for the statusbar-background ;) */
  
  /* These are used to work out how to redraw in fullscreen mode */
  gint win_ox, win_oy, win_ow, win_oh; /* co-ordinates currently drawn at */
  gint text_ow, text_oh; /* old size of the statusbar */
  int statusbar_was_on; /* true if statusbar was visible last frame */
  int idx;				/* index of image in the serie */
} scansreader_image;

typedef struct _scansreader_deletedfile {
    char *filename, *trashfile;
    int pos;
} scansreader_deletedfile;

extern int		first;
extern char		infotext[BUF_LEN];
extern GdkCursor	*cursor, *visible_cursor, *invisible_cursor;
extern GMainLoop	*scansreader_main_loop;
extern gint		screen_x, screen_y;
extern GdkGC		*black_gc;
extern GdkGC		*status_gc;
extern GdkFont		*text_font;
extern GdkColormap	*cmap;
extern char		*image_bg_spec;
extern GdkColor		image_bg;
extern GdkColor		text_bg;
extern GdkColor		error_bg;
extern int		images;
extern char		**image_names;
extern int		image_idx;
extern int		max_image_cnt;
extern scansreader_deletedfile	*deleted_files;
extern int		delete_idx;

extern int		filter;
extern gint		center;
extern gint		default_brightness;
extern gint		default_contrast;
extern gint		default_gamma;
extern gint		delay;
extern int		random_order;
extern int		random_replace;
extern int		maxpect;
extern int		statusbar_fullscreen;
extern int		statusbar_window;
extern int		slide;
extern int		scale_down;
extern int		transparency;
extern int		do_grab;
extern int		max_rand_num;
extern int		fixed_window_size;
extern int		fixed_zoom_factor;
extern int		zoom_factor;
extern int              recursive;

extern char	**helpstrs, **helpkeys;
extern const char  *image_extensions[];

/* main.c */

extern void scansreader_g_exit(int, int);
extern void scansreader_exit(int);
extern void scansreader_load_image();
extern GdkImlibImage *next_im;
extern int next_image_idx;
extern int locked_params_p;
extern struct _GdkImlibColorModifier locked_params;

/* image.c */

/* Modes for update_image */
#define REDRAW 0
#define MOVED  1
#define ZOOMED 2
#define FULL_REDRAW 3

extern void scansreader_load_image(scansreader_image *);
extern void set_desktop_image(scansreader_image *);
extern void zoom_in(scansreader_image *);
extern void zoom_out(scansreader_image *);
extern void zoom_maxpect(scansreader_image *);
extern void reload_image(scansreader_image *q);
extern void reset_coords(scansreader_image *);
extern void check_size(scansreader_image *, gint);
extern void render_to_pixmap(scansreader_image *, double *);
extern void update_image(scansreader_image *, int);
extern void update_m_image(scansreader_image *);
extern void update_z_image(scansreader_image *);
extern void reset_mod(scansreader_image *);
extern void destroy_image(scansreader_image *q);
extern void center_image(scansreader_image *q);

/* event.c */

extern void scansreader_handle_event(GdkEvent *, gpointer);

/* options.c */

extern void options_read(int, char **, scansreader_image *);

/* utils.c */

extern int  move2trash(void);
extern int  undelete_image(void);
extern void jump2image(char *);
extern void run_command(scansreader_image *, int, char *);
extern void finish(int);
extern void next_image(int);
extern void usage(char *, int);
extern void show_help(char *, int);
extern int get_random(int, int, int);
extern gboolean color_alloc(const char *, GdkColor *);
extern void swap(int *, int *);
extern int round(double);
extern int keyval_main();
extern int keyval_add();
int vchdir(char *);
void vchdir_set_pwd(void);
char *scansreader_tmpdir(void);
void scansreader_tmpdir_clean(void);
char *dir_hash_code(char *);
void shellquote(char *, char *);
int match(char *, char *, int);

extern char vchdir_pwd[];

/* help.c */
extern char *helpstrs_us[];
extern char *helpstrs_fr[];
