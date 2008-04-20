
#include <stdlib.h>
#include <iostream>
#include <cassert>
#include <zlib.h>

/**
 * Inflates either zlib or gzip deflated memory. The inflated memory is
 * expected to be freed by the caller.
 */
int
inflateMemory(unsigned char *in, unsigned int inLength,
              unsigned char *&out, unsigned int &outLength)
{
    int bufferSize = 256 * 1024;
    int ret;
    z_stream strm;

    out = (unsigned char*) malloc(bufferSize);

    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.next_in = in;
    strm.avail_in = inLength;
    strm.next_out = out;
    strm.avail_out = bufferSize;

    ret = inflateInit2(&strm, 15 + 32);

    if (ret != Z_OK)
        return ret;

    do
    {
        if (strm.next_out == NULL)
        {
            inflateEnd(&strm);
            return Z_MEM_ERROR;
        }

        ret = inflate(&strm, Z_NO_FLUSH);
        assert(ret != Z_STREAM_ERROR);

        switch (ret) {
            case Z_NEED_DICT:
                ret = Z_DATA_ERROR;
            case Z_DATA_ERROR:
            case Z_MEM_ERROR:
                (void) inflateEnd(&strm);
                return ret;
        }

        if (ret != Z_STREAM_END)
        {
            out = (unsigned char*) realloc(out, bufferSize * 2);

            if (out == NULL)
            {
                inflateEnd(&strm);
                return Z_MEM_ERROR;
            }

            strm.next_out = out + bufferSize;
            strm.avail_out = bufferSize;
            bufferSize *= 2;
        }
    }
    while (ret != Z_STREAM_END);
    assert(strm.avail_in == 0);

    outLength = bufferSize - strm.avail_out;
    (void) inflateEnd(&strm);
    return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}

int
inflateMemory(unsigned char *in, unsigned int inLength,
              unsigned char *&out)
{
    unsigned int outLength = 0;
    int ret = inflateMemory(in, inLength, out, outLength);

    if (ret != Z_OK || out == NULL)
    {
        if (ret == Z_MEM_ERROR)
        {
            std::cerr<<"Error: Out of memory while decompressing map data!";
        }
        else if (ret == Z_VERSION_ERROR)
        {
            std::cerr<<"Error: Incompatible zlib version!";
        }
        else if (ret == Z_DATA_ERROR)
        {
            std::cerr<<"Error: Incorrect zlib compressed data!";
        }
        else
        {
            std::cerr<<"Error: Unknown error while decompressing map data!";
        }

        free(out);
        out = NULL;
        outLength = 0;
    }

    return outLength;
}

/*
int
compressMemory(unsigned char *in, unsigned int inLength,
              unsigned char *&out, unsigned int &outLength)
{
    uLongf fOutLen = outLength;
    int ret = compress((Bytef*)out, &fOutLen, (Bytef*)in, inLength);
    outLength = fOutLen;

    assert (ret == Z_OK);
}
*/
