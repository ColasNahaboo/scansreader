/*
  Module       : event.c
  Purpose      : Handle GDK events
  More         : see scansreader README
  Policy       : GNU GPL
  Homepage     : http://www.klografx.net/scansreader/
*/	

#include <stdio.h>
#include <string.h>
#include <gdk/gdkx.h>
#include <gdk/gdkkeysyms.h>
#include "scansreader.h"
#include <math.h>

static int	jumping;
static char	jcmd[100];
static int	jidx;

void scansreader_scrolldown(scansreader_image *, int sf);
void scansreader_scrollup(scansreader_image *, int sf);
void scansreader_scrolldown_step(scansreader_image *, int);
void scansreader_scrollup_step(scansreader_image *, int);
void scansreader_scrollleft(scansreader_image *, int sf);
void scansreader_scrollright(scansreader_image *, int sf);
void statusbar_toggle (scansreader_image *);
void help (scansreader_image *);
void do_the_right_thing (scansreader_image *);
extern void show_at_top(scansreader_image *, int);
extern void show_at_bottom(scansreader_image *, int);
void mouse_init(scansreader_image *);
void mouse_reset(scansreader_image *);
void previous_place(int step);

static double mouse_o;
static double mouse_threshold = 2;
static double fine_scroll_mult = 2.0;
static int screen_y2;

void
scansreader_handle_event(GdkEvent * ev, gpointer data)
{
  gboolean    exit_slideshow = FALSE;
  scansreader_image  *q = data;
  static guint32 last_key_time;
  double mouse_d;

  switch (ev->type) {
  case GDK_BUTTON_PRESS:
    jumping = 0;		/* abort jump mode if a button is pressed */
    break;

  case GDK_MOTION_NOTIFY:
    mouse_d = ev->motion.y - mouse_o;
    if (fabs(mouse_d) >= mouse_threshold) {
      if (ev->motion.y > mouse_o) {
	scansreader_scrollup_step(q, (int) (ev->motion.y - mouse_o) 
			  * fine_scroll_mult);
      } else {
	scansreader_scrolldown_step(q, (int) (mouse_o - ev->motion.y) 
			    * fine_scroll_mult);
      }
      mouse_reset(q);
    }
    break;

    /* Use release instead of press (Fixes bug with junk being sent to 
     * underlying xterm window on exit) */
  case GDK_BUTTON_RELEASE:
    exit_slideshow = TRUE;
    switch (ev->button.button) {
    case 1:		/* 1st or 5th button pressed */
      next_image(1);
      scansreader_load_image(q);
      break;
    case 2:		/* 2nd button pressed */
      statusbar_toggle(q);
      if (statusbar_fullscreen) {
	help(q);
      } else {
	update_image(q, REDRAW);
      };
      break;
    case 3:
      next_image(-1);
      scansreader_load_image(q);
      break;
    case 4:		/* scroll wheel down */
      scansreader_scrolldown(q, SCROLL_FACTOR_FINE);
      break;
    case 5:		/* scroll wheel up */
      scansreader_scrollup(q, SCROLL_FACTOR_FINE);
      break;
    }
    break;

  case GDK_KEY_RELEASE:
    last_key_time = ev->key.time;
    break;

  case GDK_KEY_PRESS:
    /* avoid some bug on cygwin duplicating key events with same time
       as previous one */
    if (ev->key.time == last_key_time) break;
    last_key_time = ev->key.time;

    exit_slideshow = TRUE;	/* Abort slideshow on any key by default */
#ifdef DEBUG
    g_print("*** key: tstring: %s keyval: %d time: %d\n", 
	ev->key.string, ev->key.keyval, ev->key.time);
#endif
    if (jumping) {
      if ((ev->key.keyval == GDK_Return) || (ev->key.keyval == GDK_KP_Enter) || (jidx == 99)) {	/* 99 
												 * digits 
												 * already 
												 * typed 
												 */
	jcmd[jidx] = '\0';
	jump2image(jcmd);
	scansreader_load_image(q);
	jumping = 0;
      }
      /* else record keystroke if not null */
      else if (ev->key.string && *(ev->key.string) != '\0')
	jcmd[jidx++] = *(ev->key.string);
    } else {
      switch (ev->key.keyval) {

	/* Exit */
      case 'Q':
	scansreader_g_exit(0, 0);
	break;
      case GDK_Escape:
      case GDK_DELETE:
      case 'q':
	scansreader_g_exit(0, 1);
	break;

	/* Center mode (on/off) */

      case 'E':
      case 'e':
	exit_slideshow = FALSE;
	center ^= 1;
	snprintf(infotext, sizeof infotext, center ?
		 "(Centering: on)" : "(Centering: off)");
	update_image(q, MOVED);
	break;

	/* Previous place */
      case 'P': case 'p':
	previous_place(-1);scansreader_load_image(q);
	break;
	/* Next place */
      case 'N': case 'n':
	previous_place(1);scansreader_load_image(q);
	break;

	/* Maxpect on/off */

      case 'M':
      case 'm':
      case GDK_Tab:
	scale_down = 0;
	maxpect ^= 1;
	snprintf(infotext, sizeof infotext, maxpect ?
		 "(Maxpect: on)" : "(Maxpect: off)");
	zoom_factor = maxpect ? 0 : fixed_zoom_factor;	/* reset
							 * zoom */
	check_size(q, TRUE);
	update_image(q, REDRAW);
	break;

      case 'i':
	exit_slideshow = FALSE;
	statusbar_toggle(q);
	break;

	/* Slide show on/off */

      case 'S':
      case 's':
	exit_slideshow = FALSE;
	slide ^= 1;
	snprintf(infotext, sizeof infotext, slide ?
		 "(Slideshow: on)" : "(Slideshow: off)");
	update_image(q, REDRAW);
	break;

	/* move image right */
      case GDK_Left:
	scansreader_scrollleft(q, SCROLL_FACTOR_FINE);
	break;

	/* move image left */
      case GDK_Right:
	scansreader_scrollright(q, SCROLL_FACTOR_FINE);
	break;

	/* move image up */
      case GDK_Down:
	scansreader_scrollup(q, SCROLL_FACTOR_FINE);
	break;

	/* move image down */
      case GDK_Up:
	scansreader_scrolldown(q, SCROLL_FACTOR_FINE);
	break;

	/* Resize + */

      case GDK_KP_Add:
      case '+':
      case '=':
	snprintf(infotext, sizeof infotext, "(Zoomed in)");
	zoom_in(q);
	update_image(q, ZOOMED);
	break;

	/* Resize - */

      case GDK_KP_Subtract:
      case '-':
	snprintf(infotext, sizeof infotext, "(Zoomed out)");
	zoom_out(q);
	update_image(q, ZOOMED);
	break;

	/* Reset Image / Original (best fit) size */

      case GDK_Return:
      case GDK_KP_Enter:
	snprintf(infotext, sizeof infotext, "(Reset size)");
	reload_image(q);
	zoom_factor = fixed_zoom_factor;	/* reset zoom */
	check_size(q, TRUE);
	update_image(q, REDRAW);
	break;

	/* Next picture - or loop to the first */

      case ' ':
	do_the_right_thing (q);
	break;

	/* 5 pictures forward - or loop to the beginning */

      case GDK_Page_Down:
      case GDK_KP_Page_Down:
	snprintf(infotext, sizeof infotext,
		 "(5 pictures forward)");
	next_image(5);
	scansreader_load_image(q);
	break;

	/* Previous picture - or loop back to the last */

      case GDK_BackSpace:
	snprintf(infotext, sizeof infotext, "(Previous picture)");
	next_image(-1);
	scansreader_load_image(q);
	break;

	/* 5 pictures backward - or loop back to the last */

      case GDK_Page_Up:
      case GDK_KP_Page_Up:
	snprintf(infotext, sizeof infotext,
		 "(5 pictures backward)");
	next_image(-5);
	scansreader_load_image(q);
	break;

	/* + brightness */

      case 'B':
	snprintf(infotext, sizeof infotext, "(More brightness)");
	q->mod.brightness += 8;
	update_image(q, REDRAW);
	break;

	/* - brightness */

      case 'b':
	snprintf(infotext, sizeof infotext, "(Less brightness)");
	q->mod.brightness -= 8;
	update_image(q, REDRAW);
	break;

	/* + contrast */

      case 'C':
	snprintf(infotext, sizeof infotext, "(More contrast)");
	q->mod.contrast += 8;
	update_image(q, REDRAW);
	break;

	/* - contrast */

      case 'c':
	snprintf(infotext, sizeof infotext, "(Less contrast)");
	q->mod.contrast -= 8;
	update_image(q, REDRAW);
	break;

	/* + gamma */

      case 'G':
	snprintf(infotext, sizeof infotext, "(More gamma)");
	q->mod.gamma += 8;
	update_image(q, REDRAW);
	break;

	/* - gamma */

      case 'g':
	snprintf(infotext, sizeof infotext, "(Less gamma)");
	q->mod.gamma -= 8;
	update_image(q, REDRAW);
	break;

      case ',': delay--; if (delay < 1) delay = 1;
        break;
      case '.': delay++;
        break;

	/* Jump to image */

      case 'J':
      case 'j':
	jumping = 1;
	jidx = 0;
	break;

	/* Flip horizontal */

      case 'H':
      case 'h':
	gdk_imlib_flip_image_horizontal(q->im);
	snprintf(infotext, sizeof infotext,
		 "(Flipped horizontal)");
	update_image(q, REDRAW);
	break;

	/* Flip vertical */

      case 'V':
      case 'v':
	gdk_imlib_flip_image_vertical(q->im);
	snprintf(infotext, sizeof infotext, "(Flipped vertical)");
	update_image(q, REDRAW);
	break;

	/* Rotate right */

      case 'l': case 'L':
	gdk_imlib_rotate_image(q->im, 1);
	gdk_imlib_flip_image_horizontal(q->im);
	snprintf(infotext, sizeof infotext, "(Rotated right)");
	swap(&q->orig_w, &q->orig_h);
	swap(&q->win_w, &q->win_h);
	check_size(q, FALSE);
	update_image(q, REDRAW);
	break;

	/* Rotate left */

      case 'k': case 'K':
	gdk_imlib_rotate_image(q->im, -1);
	gdk_imlib_flip_image_vertical(q->im);
	snprintf(infotext, sizeof infotext, "(Rotated left)");
	swap(&q->orig_w, &q->orig_h);
	swap(&q->win_w, &q->win_h);
	check_size(q, FALSE);
	update_image(q, REDRAW);
	break;

	/* position in file */

      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
	{
	  int         keyval = ev->key.keyval - '0';

	  if (!keyval--)
	    keyval = 9;
	  image_idx = (keyval * (images - 1)) / 9;
	  scansreader_load_image(q);
	}
	break;
	
      case GDK_Shift_L: 
      case GDK_Shift_R: 
      case GDK_Control_L: 
      case GDK_Control_R:
      case GDK_Caps_Lock: 
      case GDK_Shift_Lock:
      case GDK_Meta_L:
      case GDK_Meta_R:
      case GDK_Alt_L:
      case GDK_Alt_R:
      case GDK_Super_L:
      case GDK_Super_R:
      case GDK_Hyper_L:
      case GDK_Hyper_R:
	break;				/* ignore modifiers */

      default:
	/* Help */
	help(q);
	break;
      }
    }
  default:
    break;
  }
  if (exit_slideshow) {
    slide = 0;
  }
}

int
step(int image_size, int screen_size, int scroll_factor) {
  int recompute = 1;
  int step = 0;
  while (recompute) {
    if (scroll_factor <= 0) {
      step = image_size - screen_size;
      recompute = 0;
    } else {
      step = (image_size - screen_size + scroll_factor - 1) / scroll_factor;
      if (step < (screen_size / 8)) {
	scroll_factor--;
      } else if (step > (screen_size / 2)) {
	scroll_factor++;
      } else {
	recompute = 0;
      }
    }
  }
  if (step < 0) step = 0;
  return step;
}

void
scansreader_scrollup(scansreader_image *q, int scroll_factor) {
  scansreader_scrollup_step(q, step(q->win_h, screen_y, scroll_factor));
}

void
scansreader_scrollup_step(scansreader_image *q, int move_step) {
    /* is image greater than screen? */
    if (q->win_h > screen_y) {
      /* bottom visible yet? */
      if (q->win_y + q->win_h > screen_y) {
	q->win_y -= move_step;
	/* sanity check */
	if (q->win_y + q->win_h < screen_y)
	  q->win_y = screen_y - q->win_h;
	snprintf(infotext, sizeof infotext, "(Moving up)");
      } else {
	show_at_bottom(q, 0);
	snprintf(infotext, sizeof infotext, "(Cannot move further up)");
      }
    } else {                      /* user is just playing around */
      /* top reached? */
      if (q->win_y > 0) {
	q->win_y -= move_step;
	/* sanity check */
	if (q->win_y < 0)
	  q->win_y = 0;
	snprintf(infotext, sizeof infotext, "(Moving up)");
      } else {
	snprintf(infotext, sizeof infotext, "(Cannot move further up)");
      }
    }
  update_image(q, MOVED);
}

void
scansreader_scrolldown(scansreader_image *q, int scroll_factor) {
  scansreader_scrolldown_step(q, step(q->win_h, screen_y, scroll_factor));
}

void
scansreader_scrolldown_step(scansreader_image *q, int move_step) {
    /* is image greater than screen? */
    if (q->win_h > screen_y) {
      /* top visible yet? */
      if (q->win_y < 0) {
	q->win_y += move_step;
	/* sanity check */
	if (q->win_y > 0)
	  q->win_y = 0;
	snprintf(infotext, sizeof infotext, "(Moving down)");
      } else {
	show_at_top(q, 0);
	snprintf(infotext, sizeof infotext, "(Cannot move further down)");
      }
	    
    } else {                      /* user is just playing around */
	    
      /* bottom reached? */
      if (q->win_y + q->win_h < screen_y) {
	q->win_y += move_step;
	/* sanity check */
	if (q->win_y + q->win_h > screen_y)
	  q->win_y = screen_y - q->win_h;
	snprintf(infotext, sizeof infotext, "(Moving down)");
      } else {
	snprintf(infotext, sizeof infotext, "(Cannot move further down)");
      }
    }
  update_image(q, MOVED);
}

void
scansreader_scrollleft(scansreader_image *q, int scroll_factor) {
  int move_step = step(q->win_w, screen_x, scroll_factor);
    /* is image greater than screen? */
    if (q->win_w > screen_x) {
      /* left border visible yet? */
      if (q->win_x < 0) {
	q->win_x += move_step;
	/* sanity check */
	if (q->win_x > 0)
	  q->win_x = 0;
	snprintf(infotext, sizeof infotext, "(Moving right)");
      } else {
	snprintf(infotext, sizeof infotext, "(Cannot move further to the right)");
      }

    } else {                      /* user is just playing around */

      /* right border reached? */
      if (q->win_x + q->win_w < screen_x) {
	q->win_x += move_step;
	/* sanity check */
	if (q->win_x + q->win_w > screen_x)
	  q->win_x = screen_x - q->win_w;
	snprintf(infotext, sizeof infotext, "(Moving right)");
      } else {
	snprintf(infotext, sizeof infotext, "(Cannot move further to the right)");
	next_image(-1);
	scansreader_load_image(q);	
	return;
      }
    }
  update_image(q, MOVED);
}

void
scansreader_scrollright(scansreader_image *q, int scroll_factor) {
  int move_step = step(q->win_w, screen_x, scroll_factor);
    /* is image greater than screen? */
    if (q->win_w > screen_x) {
      /* right border visible yet? */
      if (q->win_x + q->win_w > screen_x) {
	q->win_x -= move_step;
	/* sanity check */
	if (q->win_x + q->win_w < screen_x)
	  q->win_x = screen_x - q->win_w;
	snprintf(infotext, sizeof infotext, "(Moving left)");
      } else {
	snprintf(infotext, sizeof infotext, "(Cannot move further to the left)");
      }

    } else {                      /* user is just playing around */

      /* left border reached? */
      if (q->win_x > 0) {
	q->win_x -= move_step;
	/* sanity check */
	if (q->win_x < 0)
	  q->win_x = 0;
	snprintf(infotext, sizeof infotext, "(Moving left)");
      } else {
	snprintf(infotext, sizeof infotext, "(Cannot move further to the left)");
	next_image(1);
	scansreader_load_image(q);	
	return;
      }
    }
  update_image(q, MOVED);
}

void
statusbar_toggle (scansreader_image *q) {
    statusbar_fullscreen ^= 1;
    snprintf(infotext, sizeof infotext, statusbar_fullscreen ?
	     "(Statusbar: on)" : "(Statusbar: off)");
  update_image(q, REDRAW);

}

void
do_the_right_thing (scansreader_image *q) {
  if (q->win_w <= screen_x) {
    /* move only downwards */
    if (q->win_y + q->win_h > (screen_y + 1)) {
      scansreader_scrollup(q, SCROLL_FACTOR);
    } else {	/* we didnt move, next image! */
      next_image(1);
      scansreader_load_image(q);
    }
  } else {
    /* move right, then down */
    if (q->win_x + q->win_w > (screen_x + 1)) {
      scansreader_scrollright(q, SCROLL_FACTOR);
    } else {	/* at the right. now go down one row */
      q->win_x = 0;
      if (q->win_y + q->win_h > (screen_y + 1)) {
	scansreader_scrollup(q, SCROLL_FACTOR);
      } else {	/* we didnt move, next image! */
	next_image(1);
	scansreader_load_image(q);
      }
    }
  }
}

void
mouse_init(scansreader_image *q) {
  mouse_o = (double) screen_y / 2;
  screen_y2 = screen_y / 2;
  XWarpPointer(GDK_DISPLAY(), None, GDK_WINDOW_XWINDOW(q->win), 0, 0, 0, 0,
	       0, screen_y / 2);
}

void
mouse_reset(scansreader_image *q) {
  XWarpPointer(GDK_DISPLAY(), None, GDK_WINDOW_XWINDOW(q->win), 0, 0, 0, 0,
	       0, screen_y2);
}
