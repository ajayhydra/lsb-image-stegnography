#ifndef VIDEO_H
#define VIDEO_H
#include <stdint.h>
void start_video_stegnography(void);

typedef struct
{
    const char *password;
    size_t password_len;
    size_t password_pos;
    uint8_t size_buf[4];
    size_t size_pos;
    uint32_t extn_size;
    int extn_size_read;
    size_t extn_pos;
    char *extn;
    uint32_t data_size;
    int data_size_read;
    size_t data_pos;
    FILE *out;
    char out_name[260];
    const char *out_base;
    int error;
    int done;
} DecodeState;
typedef struct
{
    uint8_t *data;
    size_t size;
    size_t pos;
} SecretCursor;
#endif
