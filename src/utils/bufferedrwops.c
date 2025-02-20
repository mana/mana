/*
 * Buffered SDL_RWops implementation
 * Copyright (C) 2025  Thorbjørn Lindeijer
 *
 * Written as part of a conversation between a human and an AI assistant
 * on GitHub Copilot Chat.
 *
 * This code is released into the public domain. You may use it freely
 * for any purpose, including commercial applications.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include "bufferedrwops.h"

#include <string.h>

#define BUFFER_SIZE 2048

typedef struct BufferedRWops {
    SDL_RWops* source;          // The underlying RWops we're wrapping
    size_t bufferPos;           // Current position within buffer
    size_t bufferFill;          // How much valid data is in buffer
    Sint64 virtualPos;          // Current virtual stream position
    Uint8 buffer[BUFFER_SIZE];  // Our read-ahead buffer
} BufferedRWops;

static void fillBuffer(BufferedRWops* br) {
    // If there's still data in the buffer, move it to the start
    if (br->bufferPos < br->bufferFill) {
        size_t remaining = br->bufferFill - br->bufferPos;
        memmove(br->buffer, br->buffer + br->bufferPos, remaining);
        br->bufferFill = remaining;
    } else {
        br->bufferFill = 0;
    }
    br->bufferPos = 0;

    // Fill the rest of the buffer
    size_t space = BUFFER_SIZE - br->bufferFill;
    if (space > 0) {
        size_t read = SDL_RWread(br->source,
                                 br->buffer + br->bufferFill,
                                 1, space);
        br->bufferFill += read;
    }
}

static Sint64 SDLCALL buffered_size(SDL_RWops* context) {
    BufferedRWops* br = (BufferedRWops*)context->hidden.unknown.data1;
    return SDL_RWsize(br->source);
}

static Sint64 SDLCALL buffered_seek(SDL_RWops* context, Sint64 offset, int whence) {
    BufferedRWops* br = (BufferedRWops*)context->hidden.unknown.data1;
    Sint64 newPos;

    // Calculate new position
    switch (whence) {
        case RW_SEEK_SET:
            newPos = offset;
            break;
        case RW_SEEK_CUR:
            newPos = br->virtualPos + offset;
            break;
        case RW_SEEK_END: {
            Sint64 size = SDL_RWsize(br->source);
            if (size < 0) return -1;
            newPos = size + offset;
            break;
        }
        default:
            SDL_SetError("Invalid whence value");
            return -1;
    }

    if (newPos < 0) {
        SDL_SetError("Attempt to seek before start of file");
        return -1;
    }

    // Check if new position is within our buffer
    Sint64 bufferStart = br->virtualPos - br->bufferPos;
    Sint64 bufferEnd = bufferStart + br->bufferFill;

    if (newPos >= bufferStart && newPos < bufferEnd) {
        // Position is within buffer - just update buffer position
        br->bufferPos = (size_t)(newPos - bufferStart);
    } else {
        // Position is outside buffer - need to seek in source
        if (SDL_RWseek(br->source, newPos, RW_SEEK_SET) < 0) {
            return -1;
        }
        br->bufferPos = 0;
        br->bufferFill = 0;
    }

    br->virtualPos = newPos;
    return newPos;
}

static size_t SDLCALL buffered_read(SDL_RWops* context, void* dst, size_t size, size_t maxnum) {
    BufferedRWops* br = (BufferedRWops*)context->hidden.unknown.data1;
    size_t total = size * maxnum;
    size_t copied = 0;

    // Handle single byte reads separately since it is a common case when
    // SDL_mixer uses stb_vorbis.
    if (total == 1 && br->bufferPos < br->bufferFill) {
        *(Uint8*)dst = br->buffer[br->bufferPos];
        ++br->bufferPos;
        ++br->virtualPos;
        return 1;
    }

    while (copied < total) {
        size_t remaining = total - copied;

        // If buffer is empty or exhausted, try to fill it
        if (br->bufferPos >= br->bufferFill) {
            // If we want to read more than the buffer size, bypass the buffer
            if (remaining >= BUFFER_SIZE) {
                size_t read = SDL_RWread(br->source, dst + copied, 1, remaining);
                copied += read;
                br->virtualPos += read;
                break;  // Nothing more to read
            }

            fillBuffer(br);
            if (br->bufferFill == 0) break; // EOF or error
        }

        // Copy what we can from the buffer
        size_t available = br->bufferFill - br->bufferPos;
        size_t toCopy = remaining < available ? remaining : available;

        memcpy(dst + copied, br->buffer + br->bufferPos, toCopy);
        br->bufferPos += toCopy;
        copied += toCopy;
        br->virtualPos += toCopy;
    }

    return copied / size;
}

static size_t SDLCALL buffered_write(SDL_RWops* context, const void* ptr, size_t size, size_t num) {
    SDL_SetError("Write operations not supported on buffered read-only RWops");
    return 0;
}

static int SDLCALL buffered_close(SDL_RWops* context) {
    if (context) {
        BufferedRWops* br = (BufferedRWops*)context->hidden.unknown.data1;
        if (br) {
            if (br->source) {
                SDL_RWclose(br->source);
            }
            SDL_free(br);
        }
        SDL_FreeRW(context);
    }
    return 0;
}

SDL_RWops* createBufferedRWops(SDL_RWops* source) {
    if (!source) {
        SDL_SetError("NULL source RWops");
        return NULL;
    }

    BufferedRWops* br = (BufferedRWops*)SDL_malloc(sizeof(BufferedRWops));
    if (!br) {
        SDL_SetError("Out of memory");
        return NULL;
    }

    br->source = source;
    br->bufferPos = 0;
    br->bufferFill = 0;
    br->virtualPos = 0;

    SDL_RWops* rw = SDL_AllocRW();
    if (!rw) {
        SDL_free(br);
        return NULL;
    }

    rw->size = buffered_size;
    rw->seek = buffered_seek;
    rw->read = buffered_read;
    rw->write = buffered_write;
    rw->close = buffered_close;
    rw->hidden.unknown.data1 = br;

    return rw;
}
