#include "iotools.h"
#include "zlib.h"
#include <stdio.h>
#include <string.h>  // for strlen if compressing strings.

// Compression functions
int compress_onefeat(char * in, char * comp, int fsize) {
    // Struct init
    z_stream defstream;
    defstream.zalloc = Z_NULL;
    defstream.zfree = Z_NULL;
    defstream.opaque = Z_NULL;
    defstream.avail_in = (uInt)fsize; // size of input
    defstream.next_in = (Bytef *)in; // input char array
    defstream.avail_out = (uInt)fsize; // size of output
    defstream.next_out = (Bytef *)comp; // output char array
    
    // the actual compression work.
    deflateInit(&defstream, Z_BEST_COMPRESSION);
    deflate(&defstream, Z_FINISH);
    deflateEnd(&defstream);
    return defstream.total_out;
}

int decompress_onefeat(char * in, char * comp, int compsize, int fsize) {
    // Struct init
    z_stream infstream;
    infstream.zalloc = Z_NULL;
    infstream.zfree = Z_NULL;
    infstream.opaque = Z_NULL;
    infstream.avail_in = (uInt)compsize; // size of input
    infstream.next_in = (Bytef *)in; // input char array
    infstream.avail_out = (uInt)fsize; // size of output
    infstream.next_out = (Bytef *)comp; // output char array
    
    // the actual de-compression work.
    inflateInit(&infstream);
    inflate(&infstream, Z_NO_FLUSH);
    inflateEnd(&infstream);
    return infstream.total_out;
}

// File reading functions
std::ifstream::pos_type filesize(std::string filename)
{
	std::ifstream in(filename, std::ios::ate | std::ios::binary);
	return in.tellg(); 
}

int get_file_pos(int * accum, int query, int & res)
{
    int file_id = 0;    
    while (query >= accum[file_id])
    {
        file_id++;
    }
    if (!file_id)
        res = query;
    else
        res = query-accum[file_id-1];
    return file_id;
}