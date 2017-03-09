/*
 * \file tiimage.c
 *
 * \brief  Generates an image with header, load address and size
 *
 */

/*
* Copyright (C) 2010 Texas Instruments Incorporated - http://www.ti.com/
*/
/*
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions
*  are met:
*
*    Redistributions of source code must retain the above copyright
*    notice, this list of conditions and the following disclaimer.
*
*    Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the
*    documentation and/or other materials provided with the
*    distribution.
*
*    Neither the name of Texas Instruments Incorporated nor the names of
*    its contributors may be used to endorse or promote products derived
*    from this software without specific prior written permission.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
*  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
*  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
*  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
*  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
*  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
*  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
*  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
*  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
*  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
*  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


typedef struct _ti_header_
{
    unsigned int image_size;
    unsigned int load_addr;
}ti_header;


static unsigned int tiimage_swap32(unsigned int data)
{
    unsigned int result = 0;


    result  = (data & 0xFF000000) >> 24;
    result |= (data & 0x00FF0000) >> 8;
    result |= (data & 0x0000FF00) << 8;
    result |= (data & 0x000000FF) << 24;
   
    return result;
}


int main (int argc, char *argv[])
{
    FILE *in_fp, *out_fp;
    long image_size = 0;
    ti_header    hdr;
    int i = 0, len;
    unsigned int extra;
    char dummy[4] = {0,0,0,0};
    char *boot; 


    if (argc < 3)
    {
        /* expect : tiimage </path/to/boot.bin> <path/to/place/modified/boot.bin> */
        printf("Usage : \n");
        printf("tiimage takes the input image and adds the TI Image Header \n");
        printf("The resulting output is placed in the output image path\n");
        printf("Syntax: ./<executable file name> <load address> <boot mode> <input image path/name> <output image path/name>\n");
        return -1;
    }

    FILE *in_hdr = fopen("raw-mmc-header.img", "rb");
    if (!in_hdr) {
        fprintf(stderr, "could not open raw-mmc-header.img\n");
        return -1;
    }

    char buf[512];
    memset(buf, 0, sizeof(buf));
    fread(buf, 1, sizeof(buf), in_hdr);
    fclose(in_hdr);

    in_fp = fopen(argv[1], "rb+");
    if(!in_fp) {
        printf("Error opening input image file!\n");
        return -1;
    }


    out_fp = fopen(argv[2], "wb+");
    if(!out_fp) {
        printf("Error opening/creating out image file!\n");
        return -1;
    }

    // write the image header
    fwrite(buf, 1, sizeof(buf), out_fp);

    /* Calcualte the size of the input image and rewind to the begin of file */
    fseek(in_fp, 0, SEEK_END);
    image_size = ftell(in_fp);

    rewind(in_fp);
    /* Size of  new image is actual bin image size + header */
    hdr.image_size = image_size + sizeof(hdr);
    hdr.load_addr = 0x402f0400;
  

    /* Insert the header first */
    fwrite(&hdr, sizeof(hdr), 1, out_fp);

    /* Insert the actual image */
    for(i = 0; i < image_size ; i++) {
        unsigned char temp;
        fread(&temp, sizeof(temp), 1, in_fp);
        fwrite(&temp, sizeof(temp), 1, out_fp);
    }

    fclose(out_fp);
    fclose(in_fp);

    return 0;
}

