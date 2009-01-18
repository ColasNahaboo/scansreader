
#ifndef MAIN_H
#define MAIN_H

int		first = 1; /* TRUE iff this is first image */
char		infotext[BUF_LEN];
GdkCursor	*cursor, *visible_cursor, *invisible_cursor;
GMainLoop	*scansreader_main_loop;
gint		screen_x, screen_y; /* Size of the screen in pixels */
GdkFont		*text_font; /* font for statusbar and help text */
GdkColormap	*cmap; /* global colormap */
char		*image_bg_spec = IMAGE_BG;
GdkColor	image_bg; /* default background */
GdkColor	text_bg; /* statusbar and help backgrounf */
GdkColor	error_bg; /* for the error window/screen */
int		images;	/* Number of images in current collection */
char		**image_names; /* Filenames of the images */
int		image_idx; /* Index of current image displayed. 0 = 1st image */
int		max_image_cnt; /* # images currently allocated into arrays */
scansreader_deletedfile *deleted_files;
int		delete_idx;

/* Options and such */

int	filter = 1;
gint	center = 1;
gint	default_brightness = DEFAULT_BRIGHTNESS;
gint	default_contrast = DEFAULT_CONTRAST;
gint	default_gamma = DEFAULT_GAMMA;
gint	delay = SLIDE_DELAY; /* delay in slideshow mode in seconds */
int	random_order; /* TRUE if random delay in slideshow */
int	random_replace = 1; /* random with replacement by default */
int	recursive; /* TRUE if must recurse */
int	maxpect; /* TRUE if autozoom (fit-to-screen) mode */
int	statusbar_fullscreen = 1; /* TRUE if statusbar in fullscreen is turned on (default) */
int	statusbar_window = 0; /* FALSE if statusbar in window is turned off (default) */
int	slide; /* 1=slide show running */
int	scale_down; /* resize down if image x/y > screen */
int	transparency; /* transparency on/off */
int	do_grab; /* grab keboard/pointer (default off) */
int	max_rand_num; /* the largest random number range we will ask for */
int	fixed_window_size = 0; /* window width fixed size/off */
int	fixed_zoom_factor = 0; /* window fixed zoom factor (percentage)/off */
int zoom_factor = 0; /* zoom factor/off */

char *language = "us";
char **helpstrs;
/* For --help output, we'll skip the first two lines. */
char **helpkeys; /* helpstrs+2 */

/* Used for filtering */

const char *image_extensions[] = {
#ifdef EXTN_JPEG
    ".jpg",".jpeg",
#endif
#ifdef EXTN_GIF
    ".gif",
#endif
#ifdef EXTN_TIFF
    ".tif",".tiff",
#endif
#ifdef EXTN_XPM
    ".xpm",
#endif
#ifdef EXTN_XBM
    ".xbm",
#endif
#ifdef EXTN_PNG
    ".png",".pjpeg",
#endif
#ifdef EXTN_PPM
    ".ppm",
#endif
#ifdef EXTN_PNM
    ".pnm",
#endif
#ifdef EXTN_PGM
    ".pgm",
#endif
#ifdef EXTN_PCX
    ".pcx",
#endif
#ifdef EXTN_BMP
    ".bmp",
#endif
#ifdef EXTN_EIM
    ".eim",
#endif
#ifdef EXTN_TGA
    ".tga",
#endif
    NULL
};

#endif /* MAIN_H */
