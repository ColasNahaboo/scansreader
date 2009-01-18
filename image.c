/*
  Module       : image.c
  Purpose      : Routines dealing with image display
  More         : see scansreader README
  Policy       : GNU GPL
  Homepage     : http://www.klografx.net/scansreader/
*/	

#include <stdio.h>
#include <string.h>
#include <gdk/gdkx.h>
#include <sys/time.h>
#include "scansreader.h"

static void setup_win(scansreader_image *);
static void really_set_static_grav(GdkWindow *);
static int used_masks_before=0;
static struct timeval load_before, load_after;
static double load_elapsed;
void show_load_wait(scansreader_image *);
void show_at_top(scansreader_image *, int);
void show_at_bottom(scansreader_image *, int);

static int help_shown;
extern void mouse_init(scansreader_image *);

/*
 *	Load & display image
 */

void scansreader_load_image(scansreader_image *q)
{
  int next_idx = (image_idx + 1) % images;
  gettimeofday(&load_before, 0);

  if (next_idx == q->idx) {
    next_im = q->im;
    next_image_idx = q->idx;
    q->im = 0;
  }

  if (q->im) {
    /* Discard previous image. To enable caching, s/kill/destroy/. */
    if (image_idx != next_image_idx) show_load_wait(q); /* no msg for reuse */
    gdk_imlib_kill_image(q->im);
    q->im = NULL;
  }

  if (image_idx == next_image_idx) {	/* reuse prefetched image */
    q->im = next_im;
    next_im = 0;			/* avoid freeing the one in use */
  } else {
    q->im = gdk_imlib_load_image(image_names[image_idx]);
  }
  q->idx = image_idx;

  if (!q->im) { /* error */
    q->im = NULL;
    q->error = 1;
    q->orig_w = 400;
    q->orig_h = 300;
  } else { /* Retrieve image properties */
    q->error = 0;
    q->orig_w = q->im->rgb_width;
    q->orig_h = q->im->rgb_height;
  }

  check_size(q, TRUE);

  if (first) {
    setup_win(q);
    first = 0;
  }

  gdk_window_set_background(q->win, q->im ? &image_bg : &error_bg);

  gdk_keyboard_grab(q->win, FALSE, CurrentTime);
  gdk_pointer_grab(q->win, FALSE,
		   GDK_BUTTON_PRESS_MASK| GDK_BUTTON_RELEASE_MASK | 
		   GDK_POINTER_MOTION_MASK,
		   NULL, NULL, CurrentTime);
  gettimeofday(&load_after, 0);
  load_elapsed = ((load_after.tv_sec +  load_after.tv_usec / 1.0e6) -
                 (load_before.tv_sec + load_before.tv_usec / 1.0e6));

  update_image(q, FULL_REDRAW);
  XRaiseWindow(GDK_DISPLAY(), GDK_WINDOW_XWINDOW(q->win));

  /* prefetch */
  if (next_idx != next_image_idx) {
    if (next_im) gdk_imlib_kill_image(next_im);
    if (next_idx != image_idx) {
      next_im = gdk_imlib_load_image(image_names[next_idx]);
      next_image_idx = next_idx;
    } else {				/* only one image, dont load twice */
      next_im = 0;
      next_image_idx = -1;
    }
  }
}

static gchar blank_cursor[1];

static void setup_win(scansreader_image *q)
{
  GdkWindowAttr attr;
  GdkPixmap *cursor_pixmap;

    attr.window_type=GDK_WINDOW_TEMP;
    attr.wclass=GDK_INPUT_OUTPUT;
    attr.event_mask=GDK_ALL_EVENTS_MASK;
    attr.x = attr.y = 0;
    attr.width=screen_x;
    attr.height=screen_y;
    q->win = gdk_window_new(NULL, &attr, GDK_WA_X|GDK_WA_Y);
    gdk_window_show(q->win);

  q->black_gc = gdk_gc_new(q->win);
  q->status_gc = gdk_gc_new(q->win);
  gdk_gc_set_foreground(q->status_gc, &text_bg);

  cursor_pixmap = gdk_bitmap_create_from_data(q->win, blank_cursor, 1, 1);
  invisible_cursor = gdk_cursor_new_from_pixmap(cursor_pixmap, cursor_pixmap,
						&text_bg, &text_bg, 0, 0);
  gdk_window_set_cursor(q->win, invisible_cursor);
  mouse_init(q);
}

/* XXX: fix GDK. it's setting bit gravity instead of wm gravity, so we
 * have to go behind its back and kludge this ourselves. */

static void really_set_static_grav(GdkWindow *win)
{
  long dummy;

  XSizeHints *hints = XAllocSizeHints();
  XGetWMNormalHints(GDK_DISPLAY(), GDK_WINDOW_XWINDOW(win), hints, &dummy);
  hints->win_gravity = StaticGravity;
  hints->flags |= PWinGravity;
  XSetWMNormalHints(GDK_DISPLAY(), GDK_WINDOW_XWINDOW(win), hints);
  XFree(hints);
}

void zoom_in(scansreader_image *q)
{
  int zoom_percentage;

  maxpect = scale_down = 0;

  /* first compute current zoom_factor */
  if (fixed_window_size) {
    zoom_percentage=round((1.0-(q->orig_w - q->win_w)/(double)q->orig_w)*100);
    zoom_factor=(zoom_percentage - 100) / ZOOM_FACTOR;
  }

  zoom_factor++;
  if (q->win_w < (gint) screen_x) {
    zoom_factor = fixed_zoom_factor;
      /* make image fit in width */
    q->win_w = (gint) screen_x;
    q->win_h = (gint) ((q->orig_h * screen_x) / q->orig_w);
  } else {
    q->win_w = (gint)(q->orig_w * (1 + zoom_factor * ZOOM_FACTOR / 100.));
    q->win_h = (gint)(q->orig_h * (1 + zoom_factor * ZOOM_FACTOR / 100. ));
  }

  /* adapt image position
  q->win_x = (screen_x - q->win_w) / 2;
  q->win_y = (screen_y - q->win_h) / 2;
  */
}

void zoom_out(scansreader_image *q)
{
  int zoom_percentage;

  maxpect = scale_down = 0;

  /* first compute current zoom_factor */
  if (fixed_window_size) {
    zoom_percentage=round((1.0-(q->orig_w - q->win_w)/(double)q->orig_w)*100);
    zoom_factor=(zoom_percentage - 100) / ZOOM_FACTOR;
  }

  if(q->win_w > MIN(64, q->orig_w) && q->win_h > MIN(64, q->orig_h)) {
    zoom_factor--;
    if (zoom_factor <= -1 && q->orig_h > screen_y) {
      zoom_factor = -1;
      /* make image fit in full */
      q->win_w = (gint) ((q->orig_w * screen_y) / q->orig_h);
      q->win_h = (gint) screen_y;
      q->win_x = q->win_y = 0;
    } else {
      q->win_w = (gint)(q->orig_w * (1 + zoom_factor * ZOOM_FACTOR / 100.));
      q->win_h = (gint)(q->orig_h * (1 + zoom_factor * ZOOM_FACTOR / 100.));
    }

    /* adapt image position 
    q->win_x = (screen_x - q->win_w) / 2;
    q->win_y = (screen_y - q->win_h) / 2;
    */
  } else {
    snprintf(infotext, sizeof infotext, "(Can not zoom_out anymore)");
    fprintf(stderr, "scansreader: can not zoom_out anymore\n");
  }
}

void zoom_maxpect(scansreader_image *q)
{
  double zx = (double)screen_x / (double)q->orig_w;
  double zy = (double)screen_y / (double)q->orig_h;
  q->win_w = (gint)(q->orig_w * MIN(zx, zy));
  q->win_h = (gint)(q->orig_h * MIN(zx, zy));
  center_image(q);
}

/*
  Set display settings to startup values
  which are used whenever a new image is loaded.
*/

void reload_image(scansreader_image *q)
{
  gdk_imlib_destroy_image(q->im);
  q->im = gdk_imlib_load_image(image_names[image_idx]);
  q->orig_w = q->im->rgb_width;
  q->orig_h = q->im->rgb_height;
  q->win_w = (gint)(q->orig_w * (1 + zoom_factor * 0.1));
  q->win_h = (gint)(q->orig_h * (1 + zoom_factor * 0.1));
  reset_mod(q);
  center_image(q);
}

void check_size(scansreader_image *q, gint reset)
{
  if (maxpect || (scale_down && (q->orig_w>screen_x || q->orig_h>screen_y))) {
    zoom_maxpect(q);
  } else if (reset) {
    reset_coords(q);
  }

  center_image(q);
}

void reset_coords(scansreader_image *q)
{
  if (fixed_window_size) {
    double w_o_ratio = (double)(fixed_window_size) / q->orig_w;
    q->win_w = fixed_window_size;
    q->win_h = q->orig_h * w_o_ratio;
  } else {
    if (fixed_zoom_factor) {
      zoom_factor = fixed_zoom_factor; /* reset zoom */
    }
    q->win_w = (gint)(q->orig_w * (1 + zoom_factor * 0.1));
    q->win_h = (gint)(q->orig_h * (1 + zoom_factor * 0.1));
  }
}

void
update_win_title(char * win_title, int len) {
  g_snprintf(win_title, len,
	     "scansreader: %s | Page %d / %d | \"%d\" + %d clicks",
	     image_names[image_idx],
	     image_idx+1, images,
	     keyval_main(), keyval_add()
	     );
  snprintf(infotext, sizeof infotext, "(-)");
}

/* Something changed the image.  Redraw it. */

void update_image(scansreader_image *q, int mode)
{
  GdkPixmap *m = NULL;
  gint text_len, text_w, text_h;
  gchar win_title[BUF_LEN];
  double elapsed;
  struct timeval before, after;

  if (q->error) {
    g_snprintf(win_title, sizeof win_title,
        "scansreader: ERROR! cannot load image: %s", image_names[image_idx]);
    gdk_beep();
  } else {
    if (mode == REDRAW || mode == FULL_REDRAW) {
      gdk_imlib_set_image_modifier(q->im, &q->mod);
      gdk_imlib_changed_image(q->im);
    }

    if (mode == MOVED) {
      if (transparency && used_masks_before) {
        /* there should be a faster way to update the mask, but how? */
        gdk_imlib_render(q->im, q->win_w, q->win_h);
        if (q->p) gdk_imlib_free_pixmap(q->p);
        q->p = gdk_imlib_move_image(q->im);
        m = gdk_imlib_move_mask(q->im);	/* creating transparency */
      }
      update_win_title(win_title, sizeof win_title);
    } else {
      /* calculate elapsed time while we render image */
      gettimeofday(&before, 0);
      gdk_imlib_render(q->im, q->win_w, q->win_h);
      gettimeofday(&after, 0);
      elapsed = ((after.tv_sec +  after.tv_usec / 1.0e6) -
                 (before.tv_sec + before.tv_usec / 1.0e6));

      if (q->p) gdk_imlib_free_pixmap(q->p);
      q->p = gdk_imlib_move_image(q->im);
      m = gdk_imlib_move_mask(q->im);	/* creating transparency */
      update_win_title(win_title, sizeof win_title);
    }
  }

  gdk_window_set_title(q->win, win_title);

  text_len = strlen(win_title);
  text_w = gdk_text_width(text_font, win_title, text_len);
  text_h = text_font->ascent + text_font->descent;

    if (mode == FULL_REDRAW)
      gdk_window_clear(q->win);
    else {
      if (q->win_x > q->win_ox)
        gdk_draw_rectangle(q->win, q->black_gc, 1,
          q->win_ox, q->win_oy, q->win_x - q->win_ox, q->win_oh);
      if (q->win_y > q->win_oy)
        gdk_draw_rectangle(q->win, q->black_gc, 1,
          q->win_ox, q->win_oy, q->win_ow, q->win_y - q->win_oy);
      if (q->win_x + q->win_w < q->win_ox + q->win_ow)
        gdk_draw_rectangle(q->win, q->black_gc, 1,
          q->win_x + q->win_w, q->win_oy, q->win_ox + q->win_ow, q->win_oh);
      if (q->win_y + q->win_h < q->win_oy + q->win_oh)
        gdk_draw_rectangle(q->win, q->black_gc, 1,
          q->win_ox, q->win_y + q->win_h, q->win_ow, q->win_oy + q->win_oh);

      if (q->statusbar_was_on && (!statusbar_fullscreen ||
                                  q->text_ow > text_w || q->text_oh > text_h))
        gdk_draw_rectangle(q->win, q->black_gc, 1,
            screen_x-q->text_ow-9, screen_y-q->text_oh-9,
            q->text_ow+4, q->text_oh+4);
    }

    /* remove or set transparency mask */
    if (used_masks_before) {
      if (transparency)
        gdk_window_shape_combine_mask(q->win, m, q->win_x, q->win_y);
      else
        gdk_window_shape_combine_mask(q->win, 0, q->win_x, q->win_y);
    }
    else
    {
      if (transparency && m) {
        gdk_window_shape_combine_mask(q->win, m, q->win_x, q->win_y);
        used_masks_before=1;
      }
    }

    if (!q->error)
      gdk_draw_pixmap(q->win, q->black_gc, q->p, 0, 0,
                      q->win_x, q->win_y, q->win_w, q->win_h);

    if (statusbar_fullscreen) {
      gdk_draw_rectangle(q->win, q->black_gc, 0,
          screen_x-text_w-10, screen_y-text_h-10, text_w+5, text_h+5);
      gdk_draw_rectangle(q->win, q->status_gc, 1,
          screen_x-text_w-9, screen_y-text_h-9, text_w+4, text_h+4);
      gdk_draw_text(q->win, text_font, q->black_gc,
          screen_x-text_w-7, screen_y-7-text_font->descent, win_title, text_len);
    }
    show_at_top(q, 1);
    show_at_bottom(q, 1);

    q->win_ox = q->win_x;
    q->win_oy = q->win_y;
    q->win_ow = q->win_w;
    q->win_oh = q->win_h;
    q->text_ow = text_w;
    q->text_oh = text_h;
    q->statusbar_was_on = statusbar_fullscreen;

    help_shown = 0;
    gdk_flush();
}


void reset_mod(scansreader_image *q)
{
  q->mod.brightness = default_brightness;
  q->mod.contrast = default_contrast;
  q->mod.gamma = default_gamma;
}

void destroy_image(scansreader_image *q)
{
  if (q->p) gdk_imlib_free_pixmap(q->p);
  if (q->black_gc) gdk_gc_destroy(q->black_gc);
  if (q->status_gc) gdk_gc_destroy(q->status_gc);
}

void center_image(scansreader_image *q)
{
/*  q->win_x = (screen_x - q->win_w) / 2; */
/*  q->win_y = (screen_y - q->win_h) / 2; */
  q->win_x = 0;
  q->win_y = 0;
}

/* Graphic Feedbacks */
void
show_load_wait(scansreader_image *q) {
  static char *loading_text = "Loading ...";
  static int loading_text_len = 11;
  gdk_draw_rectangle(q->win, q->black_gc, 0,
          screen_x - 400, screen_y - 200, screen_x - 1, screen_y -1);
  gdk_draw_rectangle(q->win, q->status_gc, 1,
          screen_x - 399, screen_y - 199, screen_x - 2, screen_y -2);
  gdk_draw_text(q->win, text_font, q->black_gc,
          screen_x - 250, screen_y - 100, loading_text, loading_text_len);
}

void
show_at_top(scansreader_image *q, int draw) {
  static int must_draw;
  GdkPoint points_ext[4], points_int[4];

  if (draw) {
    if (must_draw) {
      points_ext[0].x = 0; points_ext[0].y = 0;
      points_ext[1].x = 25; points_ext[1].y = 0;
      points_ext[2].x = 0; points_ext[2].y = 25;
      points_ext[3].x = 0; points_ext[3].y = 0;
      points_int[0].x = 1; points_int[0].y = 1;
      points_int[1].x = 24; points_int[1].y = 1;
      points_int[2].x = 1; points_int[2].y = 24;
      points_int[3].x = 1; points_int[3].y = 1;

      gdk_draw_polygon(q->win, q->black_gc, 0, points_ext, 4);
      gdk_draw_polygon(q->win, q->status_gc, 1, points_int, 4);
    }
    must_draw = 0;
  } else {
    must_draw = 1;
  }
}
void
show_at_bottom(scansreader_image *q, int draw) {
  static int must_draw;
  GdkPoint points_ext[4], points_int[4];

  if (draw) {
    if (must_draw) {
      points_ext[0].x = screen_x-26; points_ext[0].y = screen_y-1;
      points_ext[1].x = screen_x-1; points_ext[1].y = screen_y-26;
      points_ext[2].x = screen_x-1; points_ext[2].y = screen_y-1;
      points_ext[3].x = screen_x-26; points_ext[0].y = screen_y-1;
      points_int[0].x = screen_x-2; points_int[0].y = screen_y-2;
      points_int[1].x = screen_x-24; points_int[1].y = screen_y-2;
      points_int[2].x = screen_x-2; points_int[2].y = screen_y-24;
      points_int[3].x = screen_x-2; points_int[3].y = screen_y-2;

      gdk_draw_polygon(q->win, q->black_gc, 0, points_ext, 4);
      gdk_draw_polygon(q->win, q->status_gc, 1, points_int, 4);
    }
    must_draw = 0;
  } else {
    must_draw = 1;
  }
}

void
help (scansreader_image *q) {
  gint i;
  int temp, text_w = 0, text_h;
  
  if (help_shown) {
    help_shown = 0;
    update_image(q, REDRAW);
  } else {
    
    for (i = 0; helpstrs[i]; i++) {
      temp = gdk_text_width(text_font, helpstrs[i], strlen(helpstrs[i]));
      if (text_w < temp) text_w = temp;
    }
    
    text_h = i * (text_font->ascent + text_font->descent);
    
    snprintf(infotext, sizeof infotext, "(Showing Help)");
    gdk_draw_rectangle(q->win, q->black_gc, 0,
		       screen_x/2 - text_w/2 - 4,
		       screen_y/2 - text_h/2 - 4,
		       text_w + 7, text_h + 7);
    gdk_draw_rectangle(q->win, q->status_gc, 1,
		       screen_x/2 - text_w/2 - 3,
		       screen_y/2 - text_h/2 - 3,
		       text_w + 6, text_h + 6);
    for (i = 0; helpstrs[i]; i++) {
      gdk_draw_text(q->win, text_font, q->black_gc, screen_x/2 - text_w/2,
		    screen_y/2 - text_h/2 - text_font->descent +
		    (i+1) * (text_font->ascent + text_font->descent),
		    helpstrs[i], strlen(helpstrs[i]));
    }
    help_shown = 1;
    gdk_flush();
  }
}
