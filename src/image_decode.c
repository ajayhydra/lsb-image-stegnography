/*
NAME:
DATE:
DESCRIPTION:
SAMPLE I/P:
SAMPLE O/P:
 */
#include<stdio.h>
#include"decode.h"//include user defined data types
#include"types.h"
#include<string.h>
#include"common.h"


Status do_decoding(DecodeInfo *decInfo)
{
    printf("Enter the magic string for decoding:\n");
    scanf("%99s", decInfo->decode_magic);

    if (open_files_decode(decInfo) != e_success)
    {
        puts("Failed to open files");
        return e_failure;
    }
    puts("Files opened successfully");

    if (decode_magic_string(decInfo) != e_success)
    {
        puts("Magic string decoding unsuccessful");
        return e_failure;
    }
    puts("Magic string decoded successfully");

    if (decode_extn_size(decInfo) != e_success)
    {
        puts("Extension size decoding unsuccessful");
        return e_failure;
    }
    puts("Extension size decoded successfully");

    if (decode_extension(decInfo, decInfo->extn_size) != e_success)
    {
        puts("Extension decoding unsuccessful");
        return e_failure;
    }
    puts("Extension decoded successfully");

    if (decode_secret_file_size(decInfo) != e_success)
    {
        puts("Secret file size decoding unsuccessful");
        return e_failure;
    }
    puts("Secret file size decoded successfully");

    if (decode_secretfile_data(decInfo,
                               decInfo->secret_file_size) != e_success)
    {
        puts("Secret file data decoding unsuccessful");
        return e_failure;
    }

    puts("Decoding successfully completed");
    return e_success;
}

//function for opening source image file in read mode//
Status open_files_decode( DecodeInfo *decInfo)
{
    decInfo->fptr_stego_file = fopen(decInfo->stego,"r");
    if(decInfo->fptr_stego_file == NULL)
    {
	perror("fopen");
	fprintf(stderr,"ERROR : unable to open file %s\n", decInfo->stego);
	return e_failure;
    }
    return e_success;
}

//function for decode magic string //
Status decode_magic_string(DecodeInfo *decInfo)
{
    fseek(decInfo->fptr_stego_file,54,SEEK_SET);//skip first 54 bytes which is BMP header
    char buffer[8];
    int i=0;
    // Initialize magic_char array to zeros
    memset(decInfo->magic_char, 0, sizeof(decInfo->magic_char));
    int len = strlen(decInfo->decode_magic);
    for(i=0;i<len;i++)
    {
	fread(buffer,8,1,decInfo->fptr_stego_file);//Read 8 bytes from buffer
	decode_lsb_to_data(decInfo->magic_char,buffer,i); //calling decode lsb data function
    }
    decInfo->magic_char[i] = '\0';
    if(strcmp(decInfo->decode_magic,decInfo->magic_char) == 0) //comparing two user info magic strings
    {
	return e_success;
    }
    else
    {
	return e_failure;
    }
}
//function for decoding extension size//
Status decode_extn_size(DecodeInfo *decInfo)
{
    char buffer[32];
    decInfo->extn_size=0;
    fread(buffer,32,1,decInfo->fptr_stego_file); //Read 32 bytes buffer from image file
    int i,flag=0;
    for(i=0;i<=31;i++)
    {
	if(buffer[i]  & (1<<0))
	{
	    flag=1;
	}
	else
	{
	    flag=0;
	}
	decInfo->extn_size = decInfo->extn_size |( flag << (31-i) );//decoding extension size from image data
    }
    printf("%d\n",decInfo->extn_size);
    return e_success;
}
//function for decoding extension from the image file
Status decode_extension(DecodeInfo *decInfo,int extn_size)
{
    char buffer[8];
    int i=0;
    // Initialize extension array to zeros
    memset(decInfo->extn, 0, sizeof(decInfo->extn));
    
    for(i=0;i<extn_size;i++)
    {
	fread(buffer,8,1,decInfo->fptr_stego_file);

	// Decode the LSBs of each byte in the buffer and update the extension data
	
	decode_lsb_to_data(decInfo->extn,buffer,i);
    }
    decInfo->extn[i]='\0';
    printf("File extension: %s\n",decInfo->extn);
    
     // Append decoded extension to the base filename
    strcat(decInfo->text_file, decInfo->extn);
    printf("Complete output filename: %s\n", decInfo->text_file);
    
    // Open the output text file with the complete filename
    decInfo->fptr_text_file = fopen(decInfo->text_file,"w");
     // Check for file opening errors
    if(decInfo->fptr_text_file == NULL)
    {
	perror("fopen");
	fprintf(stderr,"ERROR:unable to open file %s\n",decInfo->text_file);
	return e_failure;
    }
    printf("Output File Created: %s\n", decInfo->text_file);
    return e_success;
}
//function for decode secret file size from image file//
Status decode_secret_file_size(DecodeInfo *decInfo)
{
    decInfo->secret_file_size=0;
    char buffer[32];
    fread(buffer,32,1,decInfo->fptr_stego_file);
    // Decode the size data from the buffer and update the secret file size
    decode_size_to_data(&decInfo->secret_file_size,buffer);
    printf("%d\n",decInfo->secret_file_size);
    return e_success;
}
//function to decode secret data from image//
Status decode_secretfile_data(DecodeInfo *decInfo,int sz)
{
    char buffer[8];// Buffer to store 8 bytes of data from the stego image
    char ch=0;
    int flag=0,i,j;
    for(i=0;i<sz;i++)
    {
	 // Read 8 bytes from the stego image into the buffer
	fread(buffer,8,1,decInfo->fptr_stego_file);

	for(j=0;j < 8;j++)
	{
	    if(buffer[j] & (1<<0) )
	    {
		flag =1; // Set flag to 1 if LSB is 1
	    }
	    else
	    {
		flag = 0;// Set flag to 0 if LSB is 0
	    }
	     // Update the character 'ch' by combining the existing bits with the decoded bit
	    ch = ch | (flag << (7 - j) );
	}
	 // Write the decoded character to the text file
	fwrite(&ch,1,1,decInfo->fptr_text_file);

	ch=0; //rest ch for next itteration
    }
    return e_success;
}


