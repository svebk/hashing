#include "iotools.h"
#include "zlib.h"
#include <stdio.h>


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

void get_onefeatcomp(int query_id, size_t read_size, int* accum, vector<ifstream*>& read_in_compfeatures, vector<ifstream*>& read_in_compidx, char* feature_cp) {
    int new_pos = 0;
    int file_id = 0;
    size_t idx_size = sizeof(unsigned long long int);
    unsigned long long int start_feat,end_feat;
    char* comp_feature = new char[read_size];
    file_id = get_file_pos(accum, query_id, new_pos);
    std::cout << "Feature found in file "  << file_id << " at pos " << new_pos << std::endl;
    read_in_compidx[file_id]->seekg((unsigned long long int)(new_pos)*idx_size);
    read_in_compidx[file_id]->read((char*)&start_feat, idx_size);
    read_in_compidx[file_id]->read((char*)&end_feat, idx_size);
    read_in_compfeatures[file_id]->seekg(start_feat);
    read_in_compfeatures[file_id]->read(comp_feature, end_feat-start_feat);
    decompress_onefeat(comp_feature, feature_cp, (int)end_feat-start_feat, read_size);
    delete[] comp_feature;
}

void get_onefeat(int query_id, size_t read_size, int* accum, vector<ifstream*>& read_in_features, char* feature_cp) {
    int new_pos = 0;
    int file_id = 0;
    file_id = get_file_pos(accum, query_id, new_pos);
    std::cout << "Feature found in file "  << file_id << " at pos " << new_pos << std::endl;
    read_in_features[file_id]->seekg((unsigned long long int)(new_pos)*read_size);
    read_in_features[file_id]->read(feature_cp, read_size);    
}