//************************************************
//*                                              *
//*   TP 1&2    (c) 2017 J. FABRIZIO             *
//*                                              *
//*                               LRDE EPITA     *
//*                                              *
//************************************************

#include "image_io.hh"

#include <iostream>
#include <cstdio>

namespace tifo {

    struct struct_tga_header {
        uint8_t idl_length;      // nombre de bits du champs d'identification de l'image  commençant au bit 12h
        uint8_t color_map_type;  // 01h indique si le fichier TGA contient une palette  (contient 1 si c'est le cas , 0 sinon)
        uint8_t image_type;      // 02h contient le code du type de l'image contenue dans le fichier TGA
        uint16_t cmap_start;     // 03h defini la position de la premiére entrée de la colormap
        uint16_t cmap_length;    // 05h nombre d'éléments de la colormap
        uint8_t cmap_depth;      // 07h nombre de bits de chaque entrée de la colormap
        uint16_t x_offset;       // 08h abscisse X de l' image
        uint16_t y_offset;       // 0Ah ordonnée Y de l' image
        uint16_t width;          // 0Ch Largeur de l'image en pixels
        uint16_t height;         // 0Eh Hauteur de l'image en pixels
        uint8_t pixel_depth;     // 10h nombre de bits par pixel
        uint8_t image_descriptor_alpha_channel_bits:4; // 11h  contient 8 bits servant a décrire l'image
        uint8_t image_descriptor_image_origin:2;
        uint8_t image_descriptor_unused:2;
    }__attribute__ ((packed));


    typedef struct struct_tga_header tga_header;


/**
 * Create a default tga header for 24 bits image without colormap, with (0,0) as origin
 *
 */
    tga_header new_tga_header(int width, int height) {
        tga_header header;
        header.idl_length=0;        // nombre de bits du champs d'identification de l'image  commençant au bit 12h
        header.color_map_type=0;    // 01h indique si le fichier TGA contient une palette  (contient 1 si c'est le cas , 0 sinon)
        header.image_type=2;        // 02h contient le code du type de l'image contenue dans le fichier TGA
        header.cmap_start=0;        // 03h defini la position de la premiére entrée de la colormap
        header.cmap_length=0;       // 05h nombre d'éléments de la colormap
        header.cmap_depth=0;        // 07h nombre de bits de chaque entrée de la colormap
        header.x_offset=0;          // 08h abscisse X de l' image
        header.y_offset=0;          // 0Ah ordonnée Y de l' image
        header.width=width;     // 0Ch Largeur de l'image en pixels
        header.height=height;    // 0Eh Hauteur de l'image en pixels
        header.pixel_depth=24;      // 10h nombre de bits par pixel
        // 11h  contient 8 bits servant a décrire l'image
        header.image_descriptor_unused= 0;
        header.image_descriptor_image_origin = 0;
        header.image_descriptor_alpha_channel_bits = 0;
        return header;
    }



    bool save_image(rgb24_image &image, const char *filename) {
        tga_header header = new_tga_header(image.sx, image.sy);
        uint8_t *buffer_bgr;
        FILE *f = fopen(filename, "w");
        if (f==0) {
            std::cerr << "ERROR: can not open " << filename << " for writing!\n";
            return false;
        }

        fwrite(&header, sizeof(tga_header), 1, f);

        buffer_bgr = new uint8_t[image.length];
        for(int i = 0 ; i < image.length ; i+=3) {
            buffer_bgr[i] = image.pixels[i+2];
            buffer_bgr[i+1] = image.pixels[i+1];
            buffer_bgr[i+2] = image.pixels[i];
        }
        fwrite(buffer_bgr, 1, image.length, f);
        delete [] buffer_bgr;

        fclose(f);
        return true;
    }

    rgb24_image *load_image(const char* filename, int *width, int *height) {
        tga_header header;
        rgb24_image *image;
        uint8_t *buffer_bgr;
        FILE *f = fopen(filename, "r");
        if (f==0) {
            std::cerr << "ERROR: can not open " << filename << " for reading!\n";
            return 0;
        }

        if (fread(&header, sizeof(tga_header), 1, f)!=1) {
            std::cerr << "ERROR: can not read " << filename << "!\n";
            return 0;
        }

        if (header.pixel_depth!=24) {
            std::cerr << "ERROR: Wrong image format (not 24bits)!\n";
            return 0;
        }
        image = new rgb24_image(header.width, header.height);

        buffer_bgr = new uint8_t[image->length];
        if (fread(buffer_bgr, 1, image->length, f)!=(unsigned)image->length) {
            std::cerr << "ERROR: can not read image data!\n";
            delete image;
            return 0;
        }
        for(int i = 0 ; i < image->length ; i+=3) {
            image->pixels[i] = buffer_bgr[i+2];
            image->pixels[i+1] = buffer_bgr[i+1];
            image->pixels[i+2] = buffer_bgr[i];
        }
        delete [] buffer_bgr;

        fclose(f);
        *width = header.width;
        *height = header.height;
        return image;
    }

    gray8_image *load_gray_image(const char* filename, int *width, int *height) {
        tga_header header;
        gray8_image *image;
        uint8_t *buffer_bgr;
        FILE *f = fopen(filename, "r");
        if (f==0) {
            std::cerr << "ERROR: can not open " << filename << " for reading!\n";
            return 0;
        }

        if (fread(&header, sizeof(tga_header), 1, f)!=1) {
            std::cerr << "ERROR: can not read " << filename << "!\n";
            return 0;
        }

        if (header.pixel_depth!=8) {
            std::cerr << "ERROR: Wrong image format (not 8bits)!\n";
            return 0;
        }
        image = new gray8_image(header.width, header.height);

        buffer_bgr = new uint8_t[image->length];
        if (fread(buffer_bgr, 1, image->length, f)!=(unsigned)image->length) {
            std::cerr << "ERROR: can not read image data!\n";
            delete image;
            return 0;
        }

        for(int i = 0 ; i < image->length ; i++) {
            image->pixels[i] = buffer_bgr[i];
        }
        delete [] buffer_bgr;

        *width = header.width;
        *height = header.height;
        fclose(f);
        return image;
    }

}
