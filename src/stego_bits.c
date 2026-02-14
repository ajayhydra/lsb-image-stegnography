#include <stdio.h>
#include "types.h"
#include "common.h"


uint encode_byte_to_lsb(char data , char *buffer)
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
/* funtion for encode size to lsb */
uint encode_size_to_lsb(long size,char *buffer)
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