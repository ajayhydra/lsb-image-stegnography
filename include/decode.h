#ifndef DECODE_H
#define DECODE_H
#include"types.h"//contains user defined data types

/* 
 * Structure to store information required for
 * decoding secret file to source Image
 * Info about output and intermediate data is
 * also stored.
 */
typedef struct _DecodeInfo
{
    /* stego Image info*/
    char stego[100];
    FILE *fptr_stego_file;
    //text file info//
    char magic_char[100];
    int extn_size;
    char extn[8];
    int secret_file_size;
    char data[8];
    char text_file[100];
    FILE *fptr_text_file;
    char decode_magic[100];
}DecodeInfo;
//Declaration for read and validate arguments in cmd line//
Status read_and_validate_decode_argc(char **argv,DecodeInfo *decInfo);

//Declaration for decoding function//
Status do_decoding(DecodeInfo *decInfo);

//Declration for files opening function//
Status open_files_decode(DecodeInfo *decInfo);

//Declration for decoding magic string//
Status decode_magic_string(DecodeInfo *decInfo);

//Declration for decoding lsb data//
Status decode_lsb_to_data(char *data,char *buffer,int n);

// Declaration for decode_size_data function
uint decode_size_to_data(int *data, char *buffer);

//Declration for decoding extension size//
Status decode_extn_size(DecodeInfo *decInfo);

//Declration for decoding extensiom//
Status decode_extension(DecodeInfo *decInfo,int extn_size);

//Declaration for decoding secret file size// 
Status decode_secret_file_size(DecodeInfo *decInfo);

//Declration for decode secret data//
Status decode_secretfile_data(DecodeInfo *decInfo,int secret_file_size);

#endif



