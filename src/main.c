
#include <stdio.h>
#include <string.h>
#include "encode.h"
#include "types.h"
#include "decode.h"
#include "video.h"

int main(void)
{
    int choice;

    printf("\n");
    printf("╔══════════════════════════════════════════════╗\n");
    printf("║                                              ║\n");
    printf("║          		STEGANOGRAPHY                  ║\n");
    printf("║                                              ║\n");
    printf("╚══════════════════════════════════════════════╝\n");
    printf("\n");

    printf("┌──────────────────────────────────────────────┐\n");
    printf("│  1  Video Steganography                      │\n");
    printf("│  2  Image Steganography                      │\n");
    printf("└──────────────────────────────────────────────┘\n");

    printf("\nEnter your choice ➜ ");
    scanf("%d", &choice);

    printf("\n");

    if (choice == 1)
        start_video_stegnography();
    else if (choice == 2)
        start_image_stegnography();
    else
        printf(" Invalid choice! Please select 1 or 2.\n");

    return 0;
}