#include <stdio.h>
#include "types.h"
#include "common.h"
#include "encode.h"
#include "decode.h"


void encode_byte_to_lsb(char data , char *buffer)
{
    int i,flag;
    for(i=7;i>=0;i--)
    {
	if( data & (1<<i) )
	{
	    flag = 1;
	}
	else
	{
	    flag = 0;
	}
	buffer[7-i] = buffer[7-i] & (0xfe);
	buffer[7-i] = buffer[7-i] | (flag<<0);
    }

}
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

/* funtion for encode size to lsb */
void encode_size_to_lsb(long size,char *buffer)
{
    int i,flag;
    for(i=31;i>=0;i--)
    {
	if(size & (1<<i) )
	{
	    flag = 1;
	}
	else
	{
	    flag =0;
	}
	buffer[31-i] = buffer[31-i] & (0xfe);
	buffer[31-i]= buffer[31-i] | (flag<<0);
    }
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