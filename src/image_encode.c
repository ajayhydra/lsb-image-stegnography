#include <stdio.h>
#include "encode.h"
#include "types.h"
#include<string.h>
#include "decode.h"
#include "common.h"

/* Function Definitions */

/* Get image size
 * Input: Image file ptr
 * Output: width * height * bytes per pixel (3 in our case)
 * Description: In BMP Image, width is stored in offset 18,
 * and height after that. size is 4 bytes
 */
uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;
    // Seek to 18th byte
    fseek(fptr_image, 18, SEEK_SET);

    // Read the width (an int)
    fread(&width, sizeof(int), 1, fptr_image);
    printf("width = %u\n", width);

    // Read the height (an int)
    fread(&height, sizeof(int), 1, fptr_image);
    printf("height = %u\n", height);

    // Return image capacity
    return width * height * 3;
}

/* 
 * Get File pointers for i/p and o/p files
 * Inputs: Src Image file, Secret file and
 * Stego Image file
 * Output: FILE pointer for above files
 * Return Value: e_success or e_failure, on file errors
 */
Status open_files(EncodeInfo *encInfo)
{
    // Src Image file
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "r");
    // Do Error handling
    if (encInfo->fptr_src_image == NULL)
    {
	perror("fopen");
	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->src_image_fname);

	return e_failure;
    }

    // Secret file
    encInfo->fptr_secret = fopen(encInfo->secret_fname, "r");
    // Do Error handling
    if (encInfo->fptr_secret == NULL)
    {
	perror("fopen");
	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->secret_fname);

	return e_failure;
    }

    // Stego Image file
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "w");
    // Do Error handling
    if (encInfo->fptr_stego_image == NULL)
    {
	perror("fopen");
	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->stego_image_fname);

	return e_failure;
    }
    printf("Bmp file opned sucessfully\n");
    // No failure return e_success
    return e_success;
}

void start_image_stegnography(void)
{
    puts("Enter 1. for encoding");
    puts("Enter 2. for decoding");
    int choice;
    printf("Enter your choice:");
    scanf("%d", &choice);
    if(choice == 1)
    {
    char src_image[200];
    char secret_file[200];
    char stego_image[200];

    printf("Image Steganography\n");
    printf("Enter source image file (BMP): ");
    if (scanf("%199s", src_image) != 1)
    {
        printf("Invalid input\n");
        return;
    }
    printf("Enter secret text file: ");
    if (scanf("%199s", secret_file) != 1)
    {
        printf("Invalid input\n");
        return;
    }
    printf("Enter output stego image file (BMP, optional): ");
    if (scanf("%199s", stego_image) != 1)
    {
        printf("Invalid input\n");
        return;
    }

    EncodeInfo encInfo;
    encInfo.src_image_fname = src_image;
    encInfo.secret_fname = secret_file;
    encInfo.stego_image_fname = stego_image;

    if (do_encoding(&encInfo) == e_success)
    {
        printf("Encoding successful\n");
    }
}
    else if(choice == 2)
    {
        
        DecodeInfo decInfo;
        char output_base[100];
        printf("Image Steganography\n");
        printf("Enter stego image file (BMP): ");
        if (scanf("%199s", decInfo.stego) != 1)
        {
            printf("Invalid input\n");
            return;
        }
        printf("Enter output file base name (without extension): ");
        if (scanf("%99s", output_base) != 1)
        {
            printf("Invalid input\n");
            return;
        }
        strcpy(decInfo.text_file, output_base);
        if (do_decoding(&decInfo) == e_success)
        {
            printf("Decoding successful\n");
        }
    }

}
Status do_encoding(EncodeInfo *encInfo)
{
    printf("Enter the magic string for encoding:\n");
    scanf("%99s", encInfo->magicstring);

    /* Extract extension from secret file name */
    char *dot = strrchr(encInfo->secret_fname, '.');
    if (dot && dot != encInfo->secret_fname)
    {
        strncpy(encInfo->extn_secret_file, dot, MAX_FILE_SUFFIX - 1);
        encInfo->extn_secret_file[MAX_FILE_SUFFIX - 1] = '\0';
        printf("Secret file extension: %s\n", encInfo->extn_secret_file);
    }
    else
    {
        printf("No extension found in secret file\n");
        encInfo->extn_secret_file[0] = '\0';
    }

    if (open_files(encInfo) != e_success)
        return e_failure;

    if (check_capacity(encInfo) != e_success)
        return e_failure;
    printf("Successfully checked the capacity\n");

    if (copy_bmp_header(encInfo->fptr_src_image,
                        encInfo->fptr_stego_image) != e_success)
        return e_failure;
    printf("Copied BMP header successfully\n");

    if (encode_magic_string(encInfo->magicstring, encInfo) != e_success)
        return e_failure;
    printf("Magic string encoded successfully\n");

    if (encode_secret_file_extn_size(strlen(encInfo->extn_secret_file),
                                     encInfo) != e_success)
        return e_failure;
    printf("Secret file extension size encoded successfully\n");

    if (encode_secret_file_extn(encInfo->extn_secret_file,
                                encInfo) != e_success)
        return e_failure;
    printf("Secret file extension encoded successfully\n");

    if (encode_secret_file_size(encInfo->size_secret_file,
                                encInfo) != e_success)
        return e_failure;
    printf("Secret file size encoded successfully\n");

    if (encode_secret_file_data(encInfo) != e_success)
        return e_failure;
    printf("Secret file data encoded successfully\n");

    if (copy_remaining_img_data(encInfo->fptr_src_image,
                                encInfo->fptr_stego_image) != e_success)
        return e_failure;

    printf("Encoding successfully completed\n");
    return e_success;
}

//check capacity
Status check_capacity(EncodeInfo *encInfo)
{
    encInfo->image_capacity = get_image_size_for_bmp(encInfo->fptr_src_image);
    encInfo-> size_secret_file = get_file_size(encInfo->fptr_secret);
    if(encInfo->image_capacity > ( (strlen(encInfo->magicstring) + 4 + strlen(encInfo->extn_secret_file) + 4 + encInfo->size_secret_file ) * 8) )
    return e_success;
    
    return e_failure;
}
uint get_file_size( FILE *fptr)
{
    fseek(fptr,0,SEEK_END);
    uint tell = ftell(fptr);
    rewind(fptr);

    return tell;
}
/*copy bmp image header */
Status copy_bmp_header(FILE *fptr_src_image,FILE *stego_image_fname)
{
    rewind(fptr_src_image);
    char buffer[54];
    fread(buffer,54,1,fptr_src_image);
    fwrite(buffer,54,1,stego_image_fname);

    return e_success;
}
/* store Magic string */
Status encode_magic_string(char *magic_string,EncodeInfo *encInfo)
{
    char buffer[8];
    int len = strlen(magic_string);
    for(int i=0;i<len;i++)
    {
	fread(buffer,8,1,encInfo->fptr_src_image); 
	encode_byte_to_lsb(magic_string[i],buffer);
	fwrite(buffer,8,1,encInfo->fptr_stego_image);
    }

    return e_success;
}


/* function to encode secret file extension*/
Status encode_secret_file_extn_size(int extn_size,EncodeInfo *encInfo)
{
    char buffer[32];
    fread(buffer,32,1,encInfo->fptr_src_image);
    encode_size_to_lsb(extn_size,buffer);
    fwrite(buffer,32,1,encInfo->fptr_stego_image);
    return e_success;
}
/* encode secret file extension*/
Status encode_secret_file_extn(char *extn,EncodeInfo *encInfo)
{
    char buffer[8];
    int i;
    int len = strlen(extn);
    for(i=0;i<len;i++)
    {
	fread(buffer,8,1,encInfo->fptr_src_image); 
	encode_byte_to_lsb(extn[i],buffer);
	fwrite(buffer,8,1,encInfo->fptr_stego_image);
    }
    return e_success;
}
/* Encode secret file size*/
Status encode_secret_file_size(long file_size,EncodeInfo *encInfo)
{
    char buffer[32];
    fread(buffer,32,1,encInfo->fptr_src_image);
    encode_size_to_lsb(file_size,buffer);
    fwrite(buffer,32,1,encInfo->fptr_stego_image);
    return e_success;
}
/* encode secret file data */
Status encode_secret_file_data(EncodeInfo *encInfo)
{
    char buffer[8];
    char secret_data[encInfo->size_secret_file];
    fread(secret_data,encInfo->size_secret_file,1,encInfo->fptr_secret);
    int i;
    for(i=0;i<encInfo->size_secret_file;i++)
    {
	fread(buffer,8,1,encInfo->fptr_src_image);
	encode_byte_to_lsb(secret_data[i],buffer);
	fwrite(buffer,8,1,encInfo->fptr_stego_image);
    }
    return e_success;
}
/* copy remaining image bytes from src image to stego image after encoding*/
Status copy_remaining_img_data(FILE *fptr_src_image,FILE *fptr_stego_image)
{
    char ch;
    while(fread(&ch,1,1,fptr_src_image) > 0)
    {
	fwrite(&ch,1,1,fptr_stego_image);
    }
    return e_success;
}

