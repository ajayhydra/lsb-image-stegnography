# Video Steganography

Simple C project to hide and extract text data inside BMP images and video (AVI / MOV / MP4) files using LSB steganography.

## Features

- Encode/decode secret text into BMP images (LSB per byte).
- Encode/decode secret files into video containers (AVI, MOV/MP4) by modifying LSBs of frame data.

## Requirements

- gcc (or compatible C compiler)
- Make

## Build

From the project root run:

```bash
make
```

This produces the `stego` executable.

## Usage

Run the program and choose image or video mode:

```bash
./stego
```

Example: image encoding (interactive) — follow prompts to supply source BMP, secret text file, output file and magic string.

Example: video encoding (interactive) — supply input video (AVI/MOV/MP4), secret file, output video and password.

## Notes
- This is an educational implementation. Do not use for sensitive data without additional encryption.