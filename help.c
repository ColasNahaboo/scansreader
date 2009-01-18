#include "scansreader.h"

void usage(char *name, int exit_status)
{
    g_print("scansreader v%s - %s\n"
	"Usage: %s [options] files ...\n"
	"type '%s --help' for options.\n",
        VERSION, URL, name, name);

    scansreader_exit(exit_status);
}

void show_help(char *name, int exit_status)
{
    int i;

    g_print("scansreader v%s - %s\n"
	"Usage: %s [options] [file | directory | archive]\n\n",
        VERSION, URL, name);

    g_print(
	  "Argument can be:\n"
	  "A Directory    All the images files in it are sorted and read\n"
	  "               Does not recurse in subdirs\n"
	  "Nothing        The current directory is read\n"
	  "A file         Its directory is read\n"
	  "An archive     Its contents is read: understands .cbz, .bdz, .zip, .jar for\n"
	  "               zip files, and .cbr, bdr, .rar for rar files\n"
	  "\n"
	  "Main options:\n"
	  "    --last_pos, -l         Open last position, where we previously quit\n"
	  "    --version, -v          Print version information and exit\n"
	  "    --language=fr, -L fr   Displays french online help\n"
	  "\n"
          "Seldom used options:\n"
          "    --brightness, -b x     Set brightness to x (-32..32)\n"
          "    --center, -e           Disable window centering\n"
          "    --contrast, -c x       Set contrast to x (-32..32)\n"
          "    --do_grab, -a          Grab the pointer in windowed mode\n"
          "    --gamma, -g x          Set gamma to x (-32..32)\n"
          "    --help, -h             This help screen\n"
          "    --ignore_path_sort, -I Sort filenames by the name only\n"
          "    --maxpect, -m          Zoom to screen size and preserve aspect ratio\n"
          "    --no_filter, -n        Do not filter images by extension\n"
          "    --scale_down, -t       Shrink image(s) larger than the screen to fit\n"
          "    --transparency, -p     Enable transparency for transparent images\n"
          "    --fixed_width x, -w x  Window with fixed width x\n"
          "    --fixed_zoom x, -W x   Window with fixed zoom factor (percentage x)\n"
          "\n"
          "Slideshow options:\n"
          "    --slide, -s            Start slideshow immediately\n"
          "    --shuffle, -S          Shuffled order\n"
          "    --delay, -d x          Wait x seconds between images [default=%d]\n"
          "\n"
          "Keys:\n", SLIDE_DELAY/1000);

    /* skip header and blank line */
    for (i=0; helpkeys[i]; i++)
        g_print("    %s\n", helpkeys[i]);

    g_print("\nValid image extensions:");

    for (i=0; image_extensions[i]; i++)
	g_print("%s%s", (i%8) ? " " : "\n    ", image_extensions[i]);
    g_print("\n\n");
    
    g_print("Homepage: %s\n", URL);

    scansreader_exit(exit_status);
}

/* Used for the ? key */

char *helpstrs_us[] =
{
    "ScansReader Keys:",
    "",
    "left/right mouse      next/previous picture",
    "wheel mouse           scroll up/down",
    "middle mouse          keys help + status",
    "space                 scroll down, then next picture",       
    "backspace/right mouse previous picture",
    "PageDown / PageUp     5 pictures forward / backward",
    "left / right arrows   previous / next picture",
    "down / up arrows      scroll",
    "q / ESC / DEL         exit, saving current position",
    "p / n                 previous / next saved position",
    "Q                     exit, but do not save position",
    "S                     save position, but do not quit",
    "",
    "1,2,...,9,0        Go to image at 0, 10, 20, ..., 90, 100% of total",
    "                   Shown as pos in status (+ pages from there)",
    "+, = / -           zoom in / out (10%)",
    "TAB, m             scale to screen size on/off",
    "s                  slide show on/off",
    ", / .              faster / slower slide show (def. 10s)",
    "b / B              - / + brightness",
    "c / C              - / + contrast",
    "g / G              - / + gamma",
    "h / v              flip horizontal / vertical",
    "k / l              rotate right / left",
    "jtX<return>        jump to image number X",
    "jfX<return>        jump forward X images",
    "jbX<return>        jump backward X images",
    "enter/return       reset zoom and color settings",
    "i                  statusbar on/off:",
    "              | file name | page number | keys to get back there |",
    "?, shift or other  show keys",
    NULL
};

char *helpstrs_fr[] =
{
    "ScansReader Commandes:",
    "",
    "boutons souris gauche/droits page suivante / précedente",
    "molette de la souris         descendre / monter",
    "bouton souris du milieu      cette aide + la page courante",
    "barre d'espace               descendre, puis page suivante",       
    "backspace / bouton droit     page précedente",
    "PageDown / PageUp            avancer / reculer de 5 pages",
    "flêches droite / gauche      page suivante / précedente",
    "flêches bas / haut           descendre / monter",
    "q / ESCAPE / SUPPR           quitter en sauvant la position",
    "p / n                        precedente / suivante position sauvée",
    "Q                            quitter sans sauver",
    "S                            sauver sans quitter",
    "",
    "1,2,...,9,0        Aller à la page à 0, 10, 20, ..., 90, 100% du total",
    "                   Position donnée à la fin du status",
    "+,= / -            zoom (grossissement) plus / moins (10%)",
    "TAB, m             voir toute la page (on/off)",
    "s                  avance automatique toutes les 10 secondes (on/off)",
    ", / .              reduit / augmente de 1s le temps entre les avances",
    "b / B              - / + luminosité",
    "c / C              - / + contraste",
    "g / G              - / + gamma",
    "h / v              miroir horizontal / vertical",
    "k / l              rotation quart de tour droite / gauche",
    "jtX<return>        aller à la page numéro X",
    "jfX<return>        avancer de X pages",
    "jbX<return>        reculer de X pages",
    "enter/return       remettre aux default couleurs et zoom",
    "i                  barre de status en bas à droite (on/off):",
    "      | nom du fichier | numéro de la page | touches pour y revenir |",
    "autres touches (?, shift...)            cette aide (on/off)",
    NULL
};
