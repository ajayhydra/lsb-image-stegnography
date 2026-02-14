/*
NAME:G.AJAY REDDY
BATCH:23039D
DATE:28/2/2024
DESCRIPTION:STEGNANOGRAPHY PROJECT
*/
#include <stdio.h>
#include <string.h>
#include "encode.h"
#include "types.h"
#include "decode.h"
#include "video.h"

int main(int argc, char *argv[])
{
	printf("----STEGNOGRAPHY USING C----\n");
	int choice;
	printf("Enter 1 for video stegnography\n");
	printf("Enter 2 for image stegnography\n");
	printf("Enter your choice:");
	scanf("%d", &choice);
	if(choice == 1)
	{
		start_video_stegnography();
	}else if(choice == 2)
	{
		start_image_stegnography();
	}
	
}
