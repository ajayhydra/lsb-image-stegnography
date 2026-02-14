#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include "video.h"
#include "encode.h"

typedef enum {
    FORMAT_UNKNOWN = 0,
    FORMAT_AVI,
    FORMAT_MOV_MP4
} VideoFormat;

typedef int (*BlockCallback)(uint8_t *block, void *ctx);

static uint32_t read_u32_le(const uint8_t *p)
{
    return ((uint32_t)p[0]) |
           ((uint32_t)p[1] << 8) |
           ((uint32_t)p[2] << 16) |
           ((uint32_t)p[3] << 24);
}

static uint32_t read_u32_be(const uint8_t *p)
{
    return ((uint32_t)p[0] << 24) |
           ((uint32_t)p[1] << 16) |
           ((uint32_t)p[2] << 8) |
           ((uint32_t)p[3]);
}

static void write_u32_le(uint8_t *p, uint32_t v)
{
    p[0] = (uint8_t)(v & 0xFF);
    p[1] = (uint8_t)((v >> 8) & 0xFF);
    p[2] = (uint8_t)((v >> 16) & 0xFF);
    p[3] = (uint8_t)((v >> 24) & 0xFF);
}

static VideoFormat detect_video_format(const uint8_t *data, size_t size)
{
    if (size < 12) {
        return FORMAT_UNKNOWN;
    }
    
    // Check for AVI (RIFF container)
    if (memcmp(data, "RIFF", 4) == 0 && memcmp(data + 8, "AVI ", 4) == 0) {
        return FORMAT_AVI;
    }
    
    // Check for MOV/MP4 (starts with ftyp atom)
    if (size >= 8) {
        uint32_t atom_size = read_u32_be(data);
        if (atom_size >= 8 && atom_size <= size && memcmp(data + 4, "ftyp", 4) == 0) {
            return FORMAT_MOV_MP4;
        }
    }
    
    // Check for older QuickTime format (moov atom at start)
    if (size >= 8 && memcmp(data + 4, "moov", 4) == 0) {
        return FORMAT_MOV_MP4;
    }
    
    return FORMAT_UNKNOWN;
}

static int find_movi_list(const uint8_t *data, size_t size, size_t *movi_offset, size_t *movi_size)
{
    if (size < 12)
    {
        return 0;
    }
    if (memcmp(data, "RIFF", 4) != 0 || memcmp(data + 8, "AVI ", 4) != 0)
    {
        return 0;
    }

    size_t offset = 12;
    while (offset + 8 <= size)
    {
        const uint8_t *chunk_id = data + offset;
        uint32_t chunk_size = read_u32_le(data + offset + 4);
        offset += 8;

        if (memcmp(chunk_id, "LIST", 4) == 0)
        {
            if (offset + 4 > size || chunk_size < 4)
            {
                return 0;
            }
            const uint8_t *list_type = data + offset;
            offset += 4;

            if (memcmp(list_type, "movi", 4) == 0)
            {
                *movi_offset = offset;
                *movi_size = chunk_size - 4;
                return 1;
            }

            if (offset + (chunk_size - 4) > size)
            {
                return 0;
            }
            offset += (chunk_size - 4);
        }
        else
        {
            if (offset + chunk_size > size)
            {
                return 0;
            }
            offset += chunk_size;
        }

        if (chunk_size % 2 != 0)
        {
            offset += 1;
        }
    }

    return 0;
}

static int find_mdat_atom(const uint8_t *data, size_t size, size_t *mdat_offset, size_t *mdat_size)
{
    if (size < 8) {
        return 0;
    }
    
    size_t offset = 0;
    while (offset + 8 <= size) {
        uint32_t atom_size = read_u32_be(data + offset);
        const uint8_t *atom_type = data + offset + 4;
        
        if (atom_size < 8 || offset + atom_size > size) {
            offset += 8;
            continue;
        }
        
        if (memcmp(atom_type, "mdat", 4) == 0) {
            printf("Found MOV/MP4 mdat atom\n");
            *mdat_offset = offset + 8;
            *mdat_size = atom_size - 8;
            return 1;
        }
        
        offset += atom_size;
    }
    
    return 0;
}

static int process_chunk_blocks(uint8_t *chunk_data, size_t chunk_size, BlockCallback cb, void *ctx)
{
    size_t i = 0;
    while (i + 8 <= chunk_size)
    {
        if (!cb(chunk_data + i, ctx))
        {
            return 0;
        }
        i += 8;
    }
    return 1;
}

static int process_movi_payload(uint8_t *payload, size_t payload_size, BlockCallback cb, void *ctx)
{
    size_t offset = 0;
    while (offset + 8 <= payload_size)
    {
        uint8_t *chunk_id = payload + offset;
        uint32_t chunk_size = read_u32_le(payload + offset + 4);
        offset += 8;

        if (memcmp(chunk_id, "LIST", 4) == 0)
        {
            if (offset + 4 > payload_size || chunk_size < 4)
            {
                return 0;
            }
            offset += 4; // skip list type
            if (offset + (chunk_size - 4) > payload_size)
            {
                return 0;
            }
            if (!process_movi_payload(payload + offset, chunk_size - 4, cb, ctx))
            {
                return 0;
            }
            offset += (chunk_size - 4);
        }
        else
        {
            if (offset + chunk_size > payload_size)
            {
                return 0;
            }
            if (!process_chunk_blocks(payload + offset, chunk_size, cb, ctx))
            {
                return 0;
            }
            offset += chunk_size;
        }

        if (chunk_size % 2 != 0)
        {
            offset += 1;
        }
    }

    return 1;
}

static int count_blocks_cb(uint8_t *block, void *ctx)
{
    (void)block;
    size_t *count = (size_t *)ctx;
    (*count)++;
    return 1;
}

static size_t count_movi_blocks(uint8_t *payload, size_t payload_size)
{
    size_t count = 0;
    if (!process_movi_payload(payload, payload_size, count_blocks_cb, &count))
    {
        return 0;
    }
    return count;
}

static int encode_block_cb(uint8_t *block, void *ctx)
{
    SecretCursor *cur = (SecretCursor *)ctx;
    if (cur->pos >= cur->size)
    {
        return 1;
    }
    encode_byte_to_lsb((char)cur->data[cur->pos], (char *)block);
    cur->pos++;
    return 1;
}

static uint8_t decode_byte_from_lsb(const uint8_t *block)
{
    uint8_t ch = 0;
    for (int i = 0; i < 8; i++)
    {
        if (block[i] & 1)
        {
            ch |= (uint8_t)(1 << (7 - i));
        }
    }
    return ch;
}

static int decode_block_cb(uint8_t *block, void *ctx)
{
    DecodeState *st = (DecodeState *)ctx;
    if (st->done || st->error)
    {
        return 0;
    }

    uint8_t ch = decode_byte_from_lsb(block);

    if (st->password_pos < st->password_len)
    {
        if (ch != (uint8_t)st->password[st->password_pos])
        {
            st->error = 1;
            return 0;
        }
        st->password_pos++;
        return 1;
    }

    if (!st->extn_size_read && st->size_pos < 4)
    {
        st->size_buf[st->size_pos++] = ch;
        if (st->size_pos == 4)
        {
            st->extn_size = read_u32_le(st->size_buf);
            st->extn_size_read = 1;
            st->extn = (char *)malloc(st->extn_size + 1);
            if (st->extn == NULL)
            {
                st->error = 1;
                return 0;
            }
            st->extn_pos = 0;
            if (st->extn_size == 0)
            {
                snprintf(st->out_name, sizeof(st->out_name), "%s", st->out_base);
                st->out = fopen(st->out_name, "wb");
                if (st->out == NULL)
                {
                    st->error = 1;
                    return 0;
                }
                st->size_pos = 0;
            }
        }
        return 1;
    }

    if (st->extn_pos < st->extn_size)
    {
        st->extn[st->extn_pos++] = (char)ch;
        if (st->extn_pos == st->extn_size)
        {
            st->extn[st->extn_size] = '\0';
            snprintf(st->out_name, sizeof(st->out_name), "%s%s", st->out_base, st->extn);
            st->out = fopen(st->out_name, "wb");
            if (st->out == NULL)
            {
                st->error = 1;
                return 0;
            }
            st->size_pos = 0;
        }
        return 1;
    }

    if (!st->data_size_read && st->size_pos < 4)
    {
        st->size_buf[st->size_pos++] = ch;
        if (st->size_pos == 4)
        {
            st->data_size = read_u32_le(st->size_buf);
            st->data_size_read = 1;
            st->data_pos = 0;
            if (st->data_size == 0)
            {
                st->done = 1;
                return 0;
            }
        }
        return 1;
    }

    if (st->data_pos < st->data_size)
    {
        fwrite(&ch, 1, 1, st->out);
        st->data_pos++;
        if (st->data_pos == st->data_size)
        {
            st->done = 1;
            return 0;
        }
        return 1;
    }

    st->done = 1;
    return 0;
}

static uint8_t *read_file(const char *path, size_t *out_size)
{
    FILE *f = fopen(path, "rb");
    if (f == NULL)
    {
        return NULL;
    }
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    if (sz <= 0)
    {
        fclose(f);
        return NULL;
    }
    rewind(f);
    uint8_t *buf = (uint8_t *)malloc((size_t)sz);
    if (buf == NULL)
    {
        fclose(f);
        return NULL;
    }
    if (fread(buf, 1, (size_t)sz, f) != (size_t)sz)
    {
        free(buf);
        fclose(f);
        return NULL;
    }
    fclose(f);
    *out_size = (size_t)sz;
    return buf;
}

static int write_file(const char *path, const uint8_t *data, size_t size)
{
    FILE *f = fopen(path, "wb");
    if (f == NULL)
    {
        return 0;
    }
    if (fwrite(data, 1, size, f) != size)
    {
        fclose(f);
        return 0;
    }
    fclose(f);
    return 1;
}

static const char *get_file_ext(const char *path)
{
    const char *dot = strrchr(path, '.');
    if (dot == NULL)
    {
        return "";
    }
    return dot;
}

static int video_encode(const char *video_path, const char *secret_path, const char *out_path, const char *password)
{
    size_t video_size = 0;
    uint8_t *video_data = read_file(video_path, &video_size);
    if (video_data == NULL)
    {
        printf("Failed to read video file\n");
        return 0;
    }

    size_t movi_offset = 0;
    size_t movi_size = 0;
    
    VideoFormat fmt = detect_video_format(video_data, video_size);
    
    if (fmt == FORMAT_AVI) {
        printf("Detected AVI format\n");
        if (!find_movi_list(video_data, video_size, &movi_offset, &movi_size)) {
            printf("Could not find movi list in AVI\n");
            free(video_data);
            return 0;
        }
    } else if (fmt == FORMAT_MOV_MP4) {
        printf("Detected MOV/MP4 format\n");
        if (!find_mdat_atom(video_data, video_size, &movi_offset, &movi_size)) {
            printf("Could not find mdat atom in MOV/MP4\n");
            free(video_data);
            return 0;
        }
    } else {
        printf("Unsupported or corrupted video format\n");
        free(video_data);
        return 0;
    }
    
    size_t blocks;
    if (fmt == FORMAT_AVI) {
        blocks = count_movi_blocks(video_data + movi_offset, movi_size);
    } else {
        // For MOV/MP4, treat mdat as raw data blocks
        blocks = movi_size / 8;
    }
    if (blocks == 0)
    {
        printf("No usable movi data blocks found\n");
        free(video_data);
        return 0;
    }

    size_t secret_size = 0;
    uint8_t *secret_data = read_file(secret_path, &secret_size);
    if (secret_data == NULL)
    {
        printf("Failed to read secret file\n");
        free(video_data);
        return 0;
    }

    const char *extn = get_file_ext(secret_path);
    uint32_t extn_size = (uint32_t)strlen(extn);
    size_t password_len = strlen(password);

    size_t payload_size = password_len + 4 + extn_size + 4 + secret_size;
    if (blocks < payload_size)
    {
        printf("Insufficient capacity. Need %zu bytes, available %zu bytes\n", payload_size, blocks);
        free(secret_data);
        free(video_data);
        return 0;
    }

    uint8_t *payload = (uint8_t *)malloc(payload_size);
    if (payload == NULL)
    {
        free(secret_data);
        free(video_data);
        return 0;
    }

    size_t offset = 0;
    memcpy(payload + offset, password, password_len);
    offset += password_len;
    write_u32_le(payload + offset, extn_size);
    offset += 4;
    if (extn_size > 0)
    {
        memcpy(payload + offset, extn, extn_size);
        offset += extn_size;
    }
    write_u32_le(payload + offset, (uint32_t)secret_size);
    offset += 4;
    if (secret_size > 0)
    {
        memcpy(payload + offset, secret_data, secret_size);
        offset += secret_size;
    }

    SecretCursor cur;
    cur.data = payload;
    cur.size = payload_size;
    cur.pos = 0;
    
    if (fmt == FORMAT_AVI) {
        if (!process_movi_payload(video_data + movi_offset, movi_size, encode_block_cb, &cur))
        {
            printf("Failed while encoding AVI movi data\n");
            free(payload);
            free(secret_data);
            free(video_data);
            return 0;
        }
    } else if (fmt == FORMAT_MOV_MP4) {
        // For MOV/MP4, encode directly into mdat
        size_t i = 0;
        while (i + 8 <= movi_size && cur.pos < cur.size) {
            encode_byte_to_lsb((char)cur.data[cur.pos], (char *)(video_data + movi_offset + i));
            cur.pos++;
            i += 8;
        }
        if (cur.pos < cur.size) {
            printf("Failed while encoding MOV/MP4 mdat data\n");
            free(payload);
            free(secret_data);
            free(video_data);
            return 0;
        }
    }

    if (!write_file(out_path, video_data, video_size))
    {
        printf("Failed to write output video\n");
        free(payload);
        free(secret_data);
        free(video_data);
        return 0;
    }

    free(payload);
    free(secret_data);
    free(video_data);
    return 1;
}

static int video_decode(const char *video_path, const char *out_base, const char *password)
{
    size_t video_size = 0;
    uint8_t *video_data = read_file(video_path, &video_size);
    if (video_data == NULL)
    {
        printf("Failed to read video file\n");
        return 0;
    }

    size_t movi_offset = 0;
    size_t movi_size = 0;
    VideoFormat fmt = detect_video_format(video_data, video_size);
    
    if (fmt == FORMAT_AVI) {
        printf("Detected AVI format\n");
        if (!find_movi_list(video_data, video_size, &movi_offset, &movi_size)) {
            printf("Could not find movi list in AVI\n");
            free(video_data);
            return 0;
        }
    } else if (fmt == FORMAT_MOV_MP4) {
        printf("Detected MOV/MP4 format\n");
        if (!find_mdat_atom(video_data, video_size, &movi_offset, &movi_size)) {
            printf("Could not find mdat atom in MOV/MP4\n");
            free(video_data);
            return 0;
        }
    } else {
        printf("Unsupported or corrupted video format\n");
        free(video_data);
        return 0;
    }

    DecodeState st;
    memset(&st, 0, sizeof(st));
    st.password = password;
    st.password_len = strlen(password);
    st.out_base = out_base;
    st.extn = NULL;
    st.error = 0;
    st.done = 0;

    if (fmt == FORMAT_AVI) {
        if (!process_movi_payload(video_data + movi_offset, movi_size, decode_block_cb, &st))
        {
            if (!st.done && !st.error)
            {
                printf("Insufficient data in AVI movi payload\n");
                st.error = 1;
            }
        }
    } else if (fmt == FORMAT_MOV_MP4) {
        // For MOV/MP4, decode directly from mdat
        size_t i = 0;
        while (i + 8 <= movi_size && !st.done && !st.error) {
            if (!decode_block_cb(video_data + movi_offset + i, &st)) {
                if (!st.done && !st.error) {
                    printf("Insufficient data in MOV/MP4 mdat\n");
                    st.error = 1;
                }
                break;
            }
            i += 8;
        }
    }

    if (st.out != NULL)
    {
        fclose(st.out);
    }
    if (st.extn != NULL)
    {
        free(st.extn);
    }
    free(video_data);

    if (st.error)
    {
        printf("Decoding failed\n");
        return 0;
    }

    printf("Decoded output: %s\n", st.out_name);
    return 1;
}

void start_video_stegnography(void)
{
    int mode = 0;
    printf("Video steganography\n");
    printf("Enter 1 to encode, 2 to decode: ");
    if (scanf("%d", &mode) != 1)
    {
        printf("Invalid input\n");
        return;
    }

    if (mode == 1)
    {
        char video_file[200];
        char secret_file[200];
        char out_file[200];
        char password[100];

        printf("Enter input video file (AVI/MOV/MP4): ");
        if (scanf("%199s", video_file) != 1)
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
        printf("Enter output video file: ");
        if (scanf("%199s", out_file) != 1)
        {
            printf("Invalid input\n");
            return;
        }
        printf("Enter password: ");
        if (scanf("%99s", password) != 1)
        {
            printf("Invalid input\n");
            return;
        }

        if (video_encode(video_file, secret_file, out_file, password))
        {
            printf("Video encoding completed\n");
        }
    }
    else if (mode == 2)
    {
        char video_file[200];
        char out_base[200];
        char password[100];

        printf("Enter stego video file (AVI/MOV/MP4): ");
        if (scanf("%199s", video_file) != 1)
        {
            printf("Invalid input\n");
            return;
        }
        printf("Enter output base name (no extension): ");
        if (scanf("%199s", out_base) != 1)
        {
            printf("Invalid input\n");
            return;
        }
        printf("Enter password: ");
        if (scanf("%99s", password) != 1)
        {
            printf("Invalid input\n");
            return;
        }

        if (video_decode(video_file, out_base, password))
        {
            printf("Video decoding completed\n");
        }
    }
    else
    {
        printf("Unknown mode\n");
    }
}

