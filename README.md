# [Legacy] Scansreader 

**Warning**: This is legacy code, not maintained anymore, and depending on a discontinued [QIV](https://packages.debian.org/unstable/qiv) library by Adam Kopacz. It may however be ported to the [new QIV version](http://spiegl.de/qiv/)?



Scansreader is a "do the right thing" lean and mean unix/linux X C  program to read a set of images scanned from printed documents, such as  comics book, as confortably as possible. This page is at https://colas.nahaboo.net/Code/ScansReader

###   Goals 

 Scansreader was done to:  

-  **Be safe**, with no functionality to accidentaly modify or delete the images on disk.
-  **Be usable**, with only adapted functionalities for the task (no window mode, only fullscreen full width mode), with  streamlined interaction (scroll only vertically, without clicking the  mouse), automatic bookmarks in unlimited numbers...
-  **Be fast**, pure C, based on the fast [QIV](http://www.klografx.net/qiv/) general image viewer, with added prefetching of next image.

##   Philosophy 

 When reading a book, I do not want to be bothered having to get to the  mouse or touchpad, especially when potato-couching with my notebook, and have to aim for menus, scrollbars, buttons. Thus scansreader do not use any of this interaction method, and is always fullscreen.

It also do not provide things that belong to generic image manipulation  programs like the gimp: no thumbnails, no 2-page mode, no dialog box to  open files (you just run scansreader via your favorite desktop program).

Why the "S" in Scan*s*reader instead of the more english sounding  Scanreader? Well, there was no google hit with scansreader, but  scanreader was already the name of many products... so I went with the  less common name.

### Implementation 

 I could not find a program satisfying these criteria. All involved too  much interaction or option-setting to really read scans as painlessly as a real book.

I stumbled on a nice and fast linux image viewer [QIV](http://www.klografx.net/qiv/) by Adam Kopacz, and decided to use it as the base for scansreader,  removing a lot of its code and adding functionalities. It makes use of  the X, gdk and imlib libraries.

More details in the `README.txt` and `HISTORY.txt` files.

### License 

GPLv3

## Download & Installation 

Just compile it (`make`) and, install (`make install`) or copy the compiled `scansreader` executable in your path, e.g. in  `/usr/local/bin`.

##   User manual 

 Just run `scansreader`. it will read all the images in the  current directory, (or the directory or zip, cbz, rar or cbr archive  given as argument), and open the first one full screen, resized to the  full width of the screen.  

-  the **-l** option will re-open the last document that was opened the last time we quit the program.
-  **-L fr** will use the French language for online help texts.
-  **--help** prints the full usage, with more options

  Then, you can read your document with the following commands:  

-  **moving the mouse** will scroll the document, as **using the mousewheel**, or pressing the **up / down arrow** keys will only scroll vertically, and stop at the top and bottom of the image, indicating the end with a small yellow triangle at the bottom  right or top left of the image.
-  **SPACE** will "do the right thing": move down  the image, then go to the next one once at the bottom. If the image has  been zoomed in, and is wider than the screen, goes first to the right  then down to the left, etc...
-  **left-click** or **right arrow** key will go to next page,
-  **right-click** or **left arrow** key will go to previous page,
-  **q** or **Escape** will quit, appending the current position to the ~/.scansreader.log file
-  **p** and **n** will go to the previous or next saved place
-  **1,2,...,9,0** Goes to image at 0, 10, 20, ..., 90, 100% of current set.
-  **?** (or any other non-understood key) displays online help, with more commands



##   Other scans and comics readers 

Scansreader can be used to read digitized comics. You may want to look at similar programs: 

-  [Comic Reader](http://creader.sourceforge.net/) is similar to scansreader. In Python for linux.
-  [Powerchick](http://home.gna.org/powerchick/) another python reader
-  [Superchick](http://sacredchao.net/~piman/software.shtml) another python reader geared for manga
-  And see more of them on the [Wikipedia page](http://www.zcultfm.com/~comic/wiki/index.php/Comic_Viewers)



##   History 

- 2021-12-13 moved to GitHub fopr reference, as I remove the the mercurial site

- 2009-01-18, Wiki page + HG soureces on my public mercurial site

- v1.14

   2006-03-05, nothing changed in the code, only the packaging: 

  -  the .tgz includes now a directory
  -  compilation is now done on debian sarge (was woody)
  -  static executable is no longer included in distrib, but in        a separate tgz for a faster download of the distrib

- v1.13

   2005-02-16,  

  -  no more "random" option 
  -  TAB command to toggle between full width and full screen
  -  next image is prefetched and decoded while showing the current one for        speed
  -  Shift-Q now quits without saving in history

- v1.12

   2005-02-13,  

  -  can decode on the fly zip and rar archives

-  **v1.11** 2004-05-09, bugfix for prev/next

-  **v1.10** 2004-03-12, history file format change

-  **v1.9** 2003-12-07, p and n navigates in history

-  **v1.8** 2003-12-05, 

-  **v1.7** 2003-11-03, scrolls with mouse motions only, no need to click

-  **v1.6** 2003-11-02, french help, indicator when at top or bottom

-  **v1.5** 2003-11-01, 

-  **v1.4** 2003-10-29, 

-  **v1.3** 2003-07-02, removed windowed mode

-  **v1.2** 2003-07-01, first usable release



##   Testimonies 

"Astor" wrote (in French) on some forum after finding scansreader:

> Fran,cais: ![fr.png](https://colas.nahaboo.net/pub/System/FamFamFamFlagIcons/fr.png)
>
> scansreader est absolument génial et totalement parfait, c'est exactement ce qu'il me fallait !
>
> C'est léger, rapide, puissant, ergonomique, accessible, pratique, efficace, et en plus çà marche bien. Rien que çà. J'ai essayé sous Linux une tonne d'autres softs, mais aucun ne m'a jamais convenu, certains sont écrits en Java avec les pieds et mettent 20 secondes sur ma machine à afficher la première page d'un .cbz (scanreader est instantané), d'autres ont une interface envahissante et discutable (scansreader est plein écran, comme CDisplay, et pour moi c'est l'idéal), certains sont compliqués à installer (bon c'est bien le java mais livrer juste les .class et vas y pour la commande de lancement trouve tout seul les .jar en librairies qu'il faut, non merci).
>
> Bref, un must. Merci beaucoup à l'auteur.
>
> Merci aussi pour la version compilée en static, çà m'a bien aidé.

Here is my translation in English: ![us.png](https://colas.nahaboo.net/pub/System/FamFamFamFlagIcons/us.png)

> scansreader is absolutely awesome and totally perfect, it is exactly what I needed!
>
> It is light, fast, powerful, usable, accessible, handy, efficient, and  moreover it works well. Nothing less. I tried many other programs, but I was satisfied by none, some are  written in bad Java and take 20 seconds to show the first page of a .cbz (scansreader is instantaneous), other have a bulky and dubious  interface (scansreader, like CDisplay, is fullscreen, and for me it is  the ideal choice), some are complex to install (java is great, but  providing only the .class files and let the user figure the magic  command to launch to find the relevant .jar libraries, no thanks).
>
> In a nutshell, a must have. Thanks to the author.
>
> Thanks also for the statically compiled version, it helped me a lot.



To be honest, "Astor" later found a shortcoming of scansreader: it uses  the QIV image scaling functions that are fast but of low quality and can make noticeable artifacts (jagged lines) on some kind of pages.
