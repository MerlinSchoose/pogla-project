//************************************************
//*                                              *
//*   TP 1&2    (c) 2017 J. FABRIZIO             *
//*                                              *
//*                               LRDE EPITA     *
//*                                              *
//************************************************


#ifndef IMAGE_IO_HH
#define	IMAGE_IO_HH

#include "image.hh"

namespace tifo {

  bool save_image(rgb24_image &image, const char *filename);
  rgb24_image *load_image(const char* filename, int *width, int *height);
    gray8_image *load_gray_image(const char* filename, int *width, int *height);

}

#endif
