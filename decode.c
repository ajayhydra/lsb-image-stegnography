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
/* f4unction for to check the command line arguments read and validte it */
Status read_and_validate_decode_argc( char *argv[] , DecodeInfo *decInfo)
{
     // Check if the source image (stego image) is provided and has a .bmp extension
    if(argv[2] != NULL)
    {
	if(strcmp(strstr(argv[2],"."),".bmp") == 0)
	{
	    strcpy(decInfo->stego,argv[2]);// Copy the stego image filename
	}
	else
	{
	    return e_failure;
	}
    }
    else
    {
	return e_failure;
    }
      // Check if the text file (output file) is provided
    if(argv[3] != NULL)
    {
	 strcpy(decInfo->text_file,argv[3]);// Copy the output file filename
    }
    else
    {
	strcpy(decInfo->text_file,"text_data");// Use a default filename if not provided
    }

    

    return e_success;
}
Status do_decoding( DecodeInfo *decInfo)
{
    printf("enter the magic string for decoding:");
    scanf("%s",decInfo->decode_magic);

    if(open_files_decode(decInfo) == e_success) //condition for opening files 
    {
	puts("files opened successfully ");

	if(decode_magic_string(decInfo) == e_success) //condition for decode magic string 
	{
	    puts("magic string decoded successfully");

	    if(decode_extn_size(decInfo)==e_success) //condition for extensio size
	    {
		puts("extension size sucessfully completed");

		if(decode_extension(decInfo,decInfo->extn_size) == e_success) //condition for file extension
		{
		    puts("extension completed");

		    if(decode_secret_file_size(decInfo) == e_success) //condition for secret file size
		    {
			puts("secret size sucessful");

			if(decode_secretfile_data(decInfo,decInfo->secret_file_size) == e_success) //condition for secret data decoding 
			{
			    puts("decoding succesffuly complted");
			}
			else
			{
			    puts("decoding unsucessfully");
			    return e_failure;
			}
		    }
		    else
		    {
			puts("secret size unsuccessfully");
			return e_failure;
		    }
		}
		else
		{
		    puts("extension unsuccesful");
		    return e_failure;
		}
	    }
	    else
	    {
		puts("extension size unsucessfull");
		return e_failure;
	    }
	}
	else
	{
	    puts("magic string decoding unsucessfull");
	    return e_failure;
	}
    }
    else
    {
	puts("files opened unsuccessfull");
	return e_failure;
    }

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
//function for decode lsb into data decoding //
Status decode_lsb_to_data( char *data , char *buffer , int n)
{
    int flag=0, i;
    // Loop through each bit in a byte (from MSB to LSB)
    for(i=7;i>=0;i--)
    {
	if(buffer[i] & (1<<0) )  // Check if the least significant bit of the current byte in 'buffer' is set
	{
	    flag =1;
	}
	else
	{
	    flag = 0;
	}
	data[n] = data[n] | (flag << (7 - i) );// Update the nth element in 'data' by combining the existing bits with the decoded bit
    }
    return e_success;
}
//function for decode magic string //
Status decode_magic_string(DecodeInfo *decInfo)
{
    fseek(decInfo->fptr_stego_file,54,SEEK_CUR);//skip first 54 bytes bxz of including header files data
    char buffer[8];
    int i=0;
    decInfo->magic_char[i] = 0;
    for(i=0;i<strlen(decInfo->decode_magic);i++)
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
    for(i=0;i<extn_size;i++)
    {
	fread(buffer,8,1,decInfo->fptr_stego_file);

	// Decode the LSBs of each byte in the buffer and update the extension data
	
	decode_lsb_to_data(decInfo->extn,buffer,i);
    }
    decInfo->extn[i]='\0';
    printf("%s\n",decInfo->extn);
     // Concatenate the decoded extension to the text file name
    strcat(decInfo->text_file,decInfo->extn);

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
//function to decode the file size from data
uint decode_size_to_data(int *data, char *buffer)
{
    int flag=0, i;
    for(i=0;i<=31;i++)
    {
	if(buffer[i] & (1<<0) )
	{
	    flag =1;
	}
	else
	{
	    flag = 0;
	}
	*data = *data| (flag << (31 - i) );

    }
    return *data;
}












