#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pnmrdr.h>

#include "assert.h"
#include "a2methods.h"
#include "a2plain.h"
#include "a2blocked.h"
#include "pnm.h"

/*static A2Methods_T methods;*/
typedef A2Methods_UArray2 A2;

void apply_90(int i, int j, A2 a, void *elem, void *cl) {
    (void)elem;
    Pnm_ppm pixMap = cl;
    unsigned h = pixMap->height;
    Pnm_rgb targetLoc = pixMap->methods->at(a, i, j);
    Pnm_rgb contents = pixMap->methods->at(pixMap->pixels, j, h-i-1);
    *targetLoc = *contents;
}

void apply_180(int i, int j, A2 a, void *elem, void *cl) {
    (void)elem;
    Pnm_ppm pixMap = cl;
    unsigned h = pixMap->height;
    unsigned w = pixMap->width;
    Pnm_rgb targetLoc = pixMap->methods->at(a, i, j);
    Pnm_rgb contents = pixMap->methods->at(pixMap->pixels, w-i-1, h-j-1);
    *targetLoc = *contents;
}

void apply_270(int i, int j, A2 a, void *elem, void *cl) {
    (void)elem;
    Pnm_ppm pixMap = cl;
    unsigned w = pixMap->width;
    Pnm_rgb targetLoc = pixMap->methods->at(a, i, j);
    Pnm_rgb contents = pixMap->methods->at(pixMap->pixels, w-j-1, i);
    *targetLoc = *contents;
}

void apply_horizontal(int i, int j, A2 a, void *elem, void *cl) {
    (void)elem;
    Pnm_ppm pixMap = cl;
    unsigned w = pixMap->width;
    Pnm_rgb targetLoc = pixMap->methods->at(a, i, j);
    Pnm_rgb contents = pixMap->methods->at(pixMap->pixels, w - i - 1, j);
    *targetLoc = *contents;
}

void apply_vertical(int i, int j, A2 a, void *elem, void *cl) {
    (void)elem;
    Pnm_ppm pixMap = cl;
    unsigned h = pixMap->height;
    Pnm_rgb targetLoc = pixMap->methods->at(a, i, j);
    Pnm_rgb contents = pixMap->methods->at(pixMap->pixels, i, h -j - 1);
    *targetLoc = *contents;
}

void apply_transpose(int i, int j, A2 a, void *elem, void *cl) {
    (void)elem;
    Pnm_ppm pixMap = cl;
    Pnm_rgb targetLoc = pixMap->methods->at(a, i, j);
    Pnm_rgb contents = pixMap->methods->at(pixMap->pixels, j, i);
    *targetLoc = *contents;
}

int main(int argc, char *argv[]) {
  int rotation = 0;
  char* flip = "blank";
  A2Methods_T methods = uarray2_methods_plain; // default to UArray2 methods
  assert(methods);
  A2Methods_mapfun *map = methods->map_default; // default to best map
  assert(map);
#define SET_METHODS(METHODS, MAP, WHAT) do { \
      methods = (METHODS); \
      assert(methods); \
      map = methods->MAP; \
      if (!map) { \
        fprintf(stderr, "%s does not support " WHAT "mapping\n", argv[0]); \
        exit(1); \
      } \
    } while(0)

  int i;
  int transpose = 0;
  int stdInCheck = 0;
  FILE* fp;
  for (i = 1; i < argc; i++) {
    if (!strcmp(argv[i], "-row-major")) {
      SET_METHODS(uarray2_methods_plain, map_row_major, "row-major");
    } else if (!strcmp(argv[i], "-col-major")) {
      SET_METHODS(uarray2_methods_plain, map_col_major, "column-major");
    } else if (!strcmp(argv[i], "-block-major")) {
      SET_METHODS(uarray2_methods_blocked, map_block_major, "block-major");
    } else if (!strcmp(argv[i], "-rotate")) {
      assert(i + 1 < argc);
      char *endptr;
      // rotation holds degree number
      rotation = strtol(argv[++i], &endptr, 10);
      assert(*endptr == '\0'); // parsed all correctly
      assert(rotation == 0   || rotation == 90
          || rotation == 180 || rotation == 270);
    } else if (!strcmp(argv[i], "-flip")) {
      assert(i + 1 < argc);
        flip = argv[++i];
      assert(!strcmp(flip, "horizontal") || !strcmp(flip, "vertical"));
    } else if (!strcmp(argv[i], "-transpose")) {
      transpose = 1;
    } else if (*argv[i] == '-') {
      fprintf(stderr, "%s: unknown option '%s'\n", argv[0], argv[i]);
      exit(1);
    } else if (argc - i > 2) {
      fprintf(stderr, "Usage: %s [-rotate <angle>] "
              "[-{row,col,block}-major] [filename]\n", argv[0]);
      exit(1);
    } else if (i == argc-1) {
      fp = fopen(argv[i], "rb");
      stdInCheck = 1;
      if(fp == NULL) {
        fprintf(stderr, "Error: File does not exist\n");
        exit(1);
      }
    }
      else {
      break;
    }
  }
  if(stdInCheck==0) {
    fp = stdin;
  }

  Pnm_ppm data = Pnm_ppmread(fp, methods);
  
  A2 target;
  if(rotation == 90) {
    target = methods->new(data->height, data->width, sizeof(Pnm_rgb) * 2);
    map(target, apply_90, data);
  }
  else if(rotation == 180) {
    target = methods->new(data->width, data->height, sizeof(Pnm_rgb) * 2);
    map(target, apply_180, data);
  }
  else if(rotation == 270) {
    target = methods->new(data->height, data->width, sizeof(Pnm_rgb) * 2);
    map(target, apply_270, data);
  }  
  else if(!strcmp(flip, "horizontal")) {
    target = methods->new(data->width, data->height, sizeof(Pnm_rgb) * 2);
    map(target, apply_horizontal, data);
  }
  else if(!strcmp(flip, "vertical")) {
    target = methods->new(data->width, data->height, sizeof(Pnm_rgb) * 2);
    map(target, apply_vertical, data);
  }
  else if(transpose == 1) {
    target = methods->new(data->height, data->width, sizeof(Pnm_rgb) * 2);
    map(target, apply_transpose, data);
  }
 
  int l;
  if(rotation == 0 && strcmp(flip, "horizontal") && strcmp(flip, "vertical")
      && transpose == 0){
    Pnm_ppmwrite(stdout, data);
  }
  else{
    printf("%s\n%i %i\n%i\n", "P3", methods->width(target), 
          methods->height(target), data->denominator);
    for(l = 0; l < methods->height(target); l++) {
      for(i = 0; i < methods->width(target); i++) {
        Pnm_rgb elem = methods->at(target, i, l);
        printf("%i %i %i ", elem->red, elem->green, elem->blue);
      }
      printf("\n");
    } 
    methods->free(&target);
  }
  Pnm_ppmfree(&data);
  fclose(fp);
   //assert(0);
}
