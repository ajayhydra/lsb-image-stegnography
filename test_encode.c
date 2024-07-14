/*
NAME:G.AJAY REDDY
BATCH:23039D
DATE:28/2/2024
DESCRIPTION:STEGNANOGRAPHY PROJECT
*/
#include <stdio.h>
#include<string.h>
#include "encode.h"
#include "types.h"
#include "decode.h"
OperationType check_operation_type(char *argv[])
{
    if(strcmp(argv[1],"-e") == 0) //checking operator for encoding 
    {
	return e_encode;
    }
    else if (strcmp(argv[1],"-d") == 0) //decoding 
    {
	return e_decode;
    }
    else
    {
	return e_unsupported; //unsupported
    }
}
int main(int argc , char *argv[])
{
    if(argc >= 3) //checking command line arguments are greater than 3 or not
    {
	int res = check_operation_type(argv); //calling check operator function

	   if( res == e_encode) 
	   {
	       printf("selected encoding\n");

	       EncodeInfo encinfo; //encoding structure varriable declaration

	       if(read_and_validate_encode_args(argv,&encinfo) == e_success)
	       {
		  if( do_encoding(&encinfo) == e_success ) //encoding successful
		  {
		      printf("Encoding sucessfull\n"); 
		  }
	       }
	       else
	       {
	       puts("invalid error in encoding operator");
	       }
	   }
	   else if(res == e_decode)
	   {
	       printf("selected decoding\n");

	       DecodeInfo decInfo; //declaring decoding structure variable

	       if(read_and_validate_decode_argc(argv,&decInfo) == e_success)
	       {
		   if( do_decoding(&decInfo) == e_success) //decoding succesfull
		   {
		       puts("Decoding sucessfully complted");
		   }
	       }	   
	       else
	       {
	       puts("error in decoding operator");
	    
	       }
	   }
	   else
	   {
	       puts("Error in the command line arguments"); //printing error in the cmd line
	   }
    }
}
