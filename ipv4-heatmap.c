/*
 * IPv4 Heatmap
 * (C) 2007 The Measurement Factory, Inc
 * Licensed under the GPL, version 2
 * http://maps.measurement-factory.com/
 */

/**
 * 2016 update by Bob Rudis (@hrbrmstr)
 */

/*
 * ipv4-heatmap produces a "map" of IPv4 address space.
 *
 * input is a list of IPv4 addrs and optional value for each.
 *
 * Data is drawn using a hilbert curve, which preserves grouping see
 * http://xkcd.com/195/ and http://en.wikipedia.org/wiki/Hilbert_curve see
 * Hacker's Delight (Henry S. Warren, Jr. 2002), sec 14-2, fig 14-5
 *
 * output is a squre png or gif file
 */

#include <assert.h>
#include <err.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <gd.h>
#include <gdfonts.h>

#include "include/cidr.h"
#include "include/colors.h"

#define NUM_DATA_COLORS 256
#define RELEASE_VER "2.0.0"

#define hash_brbg 6385090210
#define hash_piyg 6385584286
#define hash_prgn 6385593500
#define hash_puor 6385597035
#define hash_rdbu 6385649970
#define hash_rdgy 6385650139
#define hash_rdylbu 6953973637431
#define hash_rdylgn 6953973637589
#define hash_spectral 7572934614486723
#define hash_bupu 6385093953
#define hash_reds 6385651123
#define hash_ylgnbu 6954256427862
#define hash_ylorbr 6954256719711
#define hash_ylorrd 6954256720225
#define hash_viridis 229486483176703

extern void annotate_file(const char *fn);
extern void shade_file(const char *fn);
extern void legend(const char *, const char *orient);

// this is the main structure to hold the heatmap
gdImagePtr image = NULL;

int colors[NUM_DATA_COLORS];
int num_colors = NUM_DATA_COLORS;
int debug = 0;
int invert = 0;

const char *whitespace = " \t\r\n";
const char *font_file_or_name = "Luxi Mono:style=Regular";
const char *legend_orient = "vert";
const char *annotations = NULL;
const char *shadings = NULL;
const char *title = NULL;
const char *legend_scale_name = NULL;

int legend_prefixes_flag = 0;
int reverse_flag = 0; /* reverse background/font colors */
int morton_flag = 0;
int accumulate_counts = 0; /* for when the input data contains a value */

struct {
  unsigned int secs;
  double input_time;
  time_t next_output;
} anim_gif = {0, 0.0, 0};

const char *legend_keyfile = NULL;
const char *palette = "viridis";
const char *savename = "map.png";
extern int annotateColor;

extern unsigned int xy_from_ip(unsigned ip, unsigned *xp, unsigned *yp);
extern void set_morton_mode();
extern int set_order();
extern void set_crop(const char *);
extern void set_bits_per_pixel(int);

void savegif(int done);

/*
 * if log_A and log_B are set, then the input data will be scaled
 * logarithmically such that log_A -> 0 and log_B -> 255. log_C is calculated
 * such that log_B -> 255.
 */
double log_A = 0.0;
double log_B = 0.0;
double log_C = 0.0;


/**
 * @brief hash function so we can use strings in switch
 */
const unsigned long hash(const char *str) {

  unsigned long hash = 5381;  
  int c;

  while ((c = *str++)) hash = ((hash << 5) + hash) + c;
  return hash;

}

/**
 * @brief Initialize gd image and hilbert colors
 */
void initialize(void) {

  int i;
  int w;
  int h;
  int order = set_order();

  w = 1 << order;
  h = 1 << order;

  if (title && 4096 != w) {
    warnx("Image width/height must be 4096 to render a legend.");
    fprintf(stderr,
            "\nIf you are using the -y or -z options, then your image size "
            "may be smaller\n(or larger) than 4096.  The legend-rendering "
            "code has a number of hard-coded\nparameters designed to work "
            "with a 4096x4096 output image.\n");
    exit(1);
  }

  if (NULL == title)
    (void)0; /* no legend */
  else if (0 == strcmp(legend_orient, "horiz"))
    h += (h >> 2);
  else
    w += (w >> 2);

  if (debug) {
    fprintf(stderr, "image width = %d\n", w);
    fprintf(stderr, "image height = %d\n", h);
  }

  image = gdImageCreateTrueColor(w, h);

  if (image == NULL) err(1, "gdImageCreateTrueColor(w=%d, h=%d)", w, h);

  /* first allocated color becomes background by default */
  if (reverse_flag) gdImageFill(image, 0, 0, gdImageColorAllocate(image, 255, 255, 255));

  int start = 0, end = NUM_DATA_COLORS, j;

  if (debug) fprintf(stderr, "invert = %d\n", invert);

  switch(hash(palette)) {
    case hash_brbg:
      for (int i=start; i<end; i++) {
        j = (invert ? NUM_DATA_COLORS - i : i);
        red[i] = brbg_red[j];
        green[i] = brbg_green[j];
        blue[i] = brbg_blue[j];
      };
      break; 
    case hash_puor:
      for (int i=start; i<end; i++) {
        j = (invert ? NUM_DATA_COLORS - i : i);
        red[i] = puor_red[j];
        green[i] = puor_green[j];
        blue[i] = puor_blue[j];
      };
      break;
    case hash_rdbu:
      for (int i=start; i<end; i++) {
        j = (invert ? NUM_DATA_COLORS - i : i);
        red[i] = rdbu_red[j];
        green[i] = rdbu_green[j];
        blue[i] = rdbu_blue[j];
      };
      break;
    case hash_rdgy:
      for (int i=start; i<end; i++) {
        j = (invert ? NUM_DATA_COLORS - i : i);
        red[i] = rdgy_red[j];
        green[i] = rdgy_green[j];
        blue[i] = rdgy_blue[j];
      };
      break;
    case hash_rdylbu:
      for (int i=start; i<end; i++) {
        j = (invert ? NUM_DATA_COLORS - i : i);
        red[i] = rdylbu_red[j];
        green[i] = rdylbu_green[j];
        blue[i] = rdylbu_blue[j];
      };
      break;
    case hash_spectral:
      for (int i=start; i<end; i++) {
        j = (invert ? NUM_DATA_COLORS - i : i);
        red[i] = spectral_red[j];
        green[i] = spectral_green[j];
        blue[i] = spectral_blue[j];
      };
      break;
    case hash_bupu:
      for (int i=start; i<end; i++) {
        j = (invert ? NUM_DATA_COLORS - i : i);
        red[i] = bupu_red[j];
        green[i] = bupu_green[j];
        blue[i] = bupu_blue[j];
      };
      break;
    case hash_reds:
      for (int i=start; i<end; i++) {
        j = (invert ? NUM_DATA_COLORS - i : i);
        red[i] = reds_red[j];
        green[i] = reds_green[j];
        blue[i] = reds_blue[j];
      };
      break;
    case hash_ylgnbu:
      for (int i=start; i<end; i++) {
        j = (invert ? NUM_DATA_COLORS - i : i);
        red[i] = ylgnbu_red[j];
        green[i] = ylgnbu_green[j];
        blue[i] = ylgnbu_blue[j];
      };
      break;
    case hash_ylorbr:
      for (int i=start; i<end; i++) {
        j = (invert ? NUM_DATA_COLORS - i : i);
        red[i] = ylorbr_red[j];
        green[i] = ylorbr_green[j];
        blue[i] = ylorbr_blue[j];
      };
      break;
    case hash_ylorrd:
      for (int i=start; i<end; i++) {
        j = (invert ? NUM_DATA_COLORS - i : i);
        red[i] = ylorrd_red[j];
        green[i] = ylorrd_green[j];
        blue[i] = ylorrd_blue[j];
      };
      break;
    default:
      for (int i=start; i<end; i++) {
        j = (invert ? NUM_DATA_COLORS - i : i);
        red[i] = viridis_red[j];
        green[i] = viridis_green[j];
        blue[i] = viridis_blue[j];
      };
      break;
  };

  for (i = 0; i < NUM_DATA_COLORS; i++) {

    colors[i] = gdImageColorAllocate(image, red[i], green[i], blue[i]);

    if (debug > 1)
      fprintf(stderr, "colors[%d]=%d {%f, %f, %f}\n", 
        i, colors[i], red[i], green[i], blue[i]);

  }

  /*
   * If the input data should be logarithmically scaled, then calculate the
   * value of log_C.
   */
  if (0.0 != log_A && 0.0 == log_B) log_B = 10.0 * log_A;

  log_C = 255.0 / log(log_B / log_A);

}

/**
 * @brief Retrieves pixel color at an x+y location in the gd image
 *
 * Used to get the color value at the current xy position.
 * @param x,y coordinates in the gd image
 */
int get_pixel_value(unsigned int x, unsigned int y) {
  int color;
  int k;
  color = gdImageGetPixel(image, x, y);
  if (debug > 1) fprintf(stderr, "[%d,%d] color idx %d\n", x, y, color);
  for (k = 0; k < NUM_DATA_COLORS; k++) {
    if (colors[k] == color) {
      if (debug > 1) fprintf(stderr, "color %d has idx %d\n", color, k);
      break;
    }
  }
  if (k == NUM_DATA_COLORS) /* not found */
    k = 0;
  return k;
}

/**
 * @brief Create the hilbert heatmap
 *
 * Reads in IP addresses and increments pixel colors.
 */
void paint(void) {

  char buf[512];
  unsigned int line = 1;

  while (fgets(buf, 512, stdin)) {

    unsigned int i;
    unsigned int x;
    unsigned int y;
    int color = -1;
    int k;
    char *strtok_arg = buf;
    char *t;

    /*
     * In animated gif mode the first field is a timestamp
     */
    if (anim_gif.secs) {
      char *e;
      t = strtok(strtok_arg, whitespace);
      strtok_arg = NULL;
      if (NULL == t)
        continue;
      anim_gif.input_time = strtod(t, &e);
      if (e == t)
        errx(1, "bad input parsing time on line %d: %s", line, t);
      if ((time_t)anim_gif.input_time > anim_gif.next_output) {
        savegif(0);
        anim_gif.next_output = (time_t)anim_gif.input_time + anim_gif.secs;
      }
    }

    /*
     * next field is an IP address.  We also accept its integer notation
     * equivalent.
     */

    t = strtok(strtok_arg, whitespace);
    strtok_arg = NULL;
    if (NULL == t) continue;

    if (strspn(t, "0123456789") == strlen(t))
      i = strtoul(t, NULL, 10);
    else if (1 == inet_pton(AF_INET, t, &i))
      i = ntohl(i);
    else
      errx(1, "bad input parsing IP on line %d: %s", line, t);

    if (0 == xy_from_ip(i, &x, &y)) continue;

    if (debug > 2) fprintf(stderr, "%s => %u => (%d,%d)\n", t, i, x, y);

    /*
     * next field is an optional value, which might also be
     * logarithmically scaled by us.  If no value is given, then find the
     * existing value at that point and increment by one.
     */
    t = strtok(NULL, whitespace);
    if (NULL != t) {

      k = atoi(t);
      if (accumulate_counts) k += get_pixel_value(x, y);
      if (0.0 != log_A) k = (int)((log_C * log((double)k / log_A)) + 0.5); // apply logarithmic stretching

    } else {

      k = get_pixel_value(x, y);
      k++;

    }

    if (k < 0) k = 0;

    if (k >= NUM_DATA_COLORS) k = NUM_DATA_COLORS - 1;

    color = colors[k];

    gdImageSetPixel(image, x, y, color);

    line++;

  }

}

/**
 * @brief Add watermark to the hilbert gd image
 */
void watermark(void) {

  int color = gdImageColorAllocateAlpha(image, 127, 127, 127, 63);

  gdImageStringUp(image, gdFontGetSmall(), gdImageSX(image) - 20, 220,
                  (u_char *)"", color);

}

/**
 * @brief Write hilbert gd image to a png file
 */
void save(void) {

  FILE *pngout = fopen(savename, "wb");

  gdImagePng(image, pngout);

  fclose(pngout);

  gdImageDestroy(image);

  image = NULL;

}

/**
 * @brief Write hilbert gd image to a gif file
 */
void savegif(int done) {

  static int ngif = 0;
  static char *tdir = NULL;
  static char tmpl[] = "heatmap-tmp-XXXXXX";
  char fname[512];
  FILE *gifout = NULL;

  if (NULL == tdir) {

    tdir = mkdtemp(tmpl);
    if (NULL == tdir) err(1, "%s", tmpl);

  }

  snprintf(fname, 512, "%s/%07d.gif", tdir, ngif++);

  gifout = fopen(fname, "wb");

  if (NULL == gifout) err(1, "%s", fname);

  gdImageGif(image, gifout);
  
  fclose(gifout);

  /* don't destroy image! */
  if (done) {

    char cmd[512];

    snprintf(cmd, 512, "gifsicle --colors 256 %s/*.gif > %s", tdir, savename);
    fprintf(stderr, "Executing: %s\n", cmd);

    if (0 != system(cmd)) errx(1, "gifsicle failed");

    snprintf(cmd, 512, "rm -rf %s", tdir);
    fprintf(stderr, "Executing: %s\n", cmd);
    
    system(cmd);
    
    tdir = NULL;
    
    gdImageDestroy(image);
    
    image = NULL;

  }

}

/**
 * @brief Display usage
 */
void usage(const char *argv0) {

  const char *t = strrchr(argv0, '/');
  printf("IPv4 Heatmap"
#ifdef RELEASE_VER
         " (release " RELEASE_VER ")"
#endif
         "\n");
  printf("(C) 2007 The Measurement Factory, Inc (GPL-2)\n");
  printf("http://maps.measurement-factory.com/\n");
  printf("Version 2 by Bob Rudis\n");
  printf("\n");
  printf("usage: %s [options] < iplist\n", t ? t + 1 : argv0);
  printf("\t-A float   logarithmic scaling, min value\n");
  printf("\t-B float   logarithmic scaling, max value\n");
  printf("\t-C         values accumulate in Exact input mode\n");
  printf("\t-a file    annotations file\n");
  printf("\t-c color   color of annotations (0xRRGGBB)\n");
  printf("\t-d         increase debugging\n");
  printf("\t-f font    fontconfig name or .ttf file\n");
  printf("\t-g secs    make animated gif from each secs of data\n");
  printf("\t-h         draw horizontal legend instead\n");
  printf("\t-i         invert the order for the selected color palette\n");
  printf("\t-k file    key file for legend\n");
  printf("\t-m         use morton order instead of hilbert\n");
  printf("\t-o file    output filename\n");
  printf("\t-P palette choose color palette (viridis, brbg puor rdbu rdgy rdylbu spectral\n\t           bupu reds ylgnbu ylorbr ylorrd). Defaults to viridis.\n");
  printf("\t-p         show size of prefixes in legend\n");
  printf("\t-r         reverse; white background, black text\n");
  printf("\t-s file    shading file\n");
  printf("\t-t str     map title\n");
  printf("\t-u str     scale title in legend\n");
  printf("\t-y cidr    address space to render\n");
  printf("\t-z bits    address space bits per pixel\n");
  printf("\t-?         display usage\n");
  exit(1);

}

// -------------------------------------------------------------------------

int main(int argc, char *argv[]) {

  int ch;

  while ((ch = getopt(argc, argv, "A:B:a:Cc:df:g:hik:mo:P:prs:t:u:y:z:?")) != -1) {

    switch (ch) {
    case 'A':
      log_A = atof(optarg);
      break;
    case 'B':
      log_B = atof(optarg);
      break;
    case 'C':
      accumulate_counts = 1;
      break;
    case 'd':
      debug++;
      break;
    case 'a':
      annotations = strdup(optarg);
      break;
    case 'c':
      annotateColor = strtol(optarg, NULL, 16);
      break;
    case 's':
      shadings = strdup(optarg);
      break;
    case 'f':
      font_file_or_name = strdup(optarg);
      break;
    case 'g':
      anim_gif.secs = strtol(optarg, NULL, 10);
      break;
    case 'h':
      legend_orient = "horiz";
      break;
    case 'i':
      invert = 1;
      break;
    case 'k':
      legend_keyfile = strdup(optarg);
      break;
    case 'o':
      savename = strdup(optarg);
      break;
    case 'm':
      morton_flag = 1;
      set_morton_mode();
      break;
    case 't':
      title = strdup(optarg);
      break;
    case 'P':
      palette = strdup(optarg);
      break;
    case 'p':
      legend_prefixes_flag = 1;
      break;
    case 'u':
      legend_scale_name = strdup(optarg);
      break;
    case 'r':
      reverse_flag = 1;
      break;
    case 'y':
      set_crop(optarg);
      break;
    case 'z':
      set_bits_per_pixel(strtol(optarg, NULL, 10));
      break;
    case '?':
    default:
      usage(argv[0]);
      break;
    }
  }

  argc -= optind;
  argv += optind;

  initialize();
  
  paint();
  
  if (shadings) shade_file(shadings);
  
  if (annotations) annotate_file(annotations);
  
  if (title) legend(title, legend_orient);
  
  watermark();
  
  if (anim_gif.secs)
    savegif(1);
  else
    save();

  return 0;

}
