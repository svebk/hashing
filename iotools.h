#include <fstream>
#include <math.h>
#include <vector>
#include <iostream>

using namespace std;

// Compression functions
int compress_onefeat(char * in, char * comp, int fsize);
int decompress_onefeat(char * in, char * comp, int compsize, int fsize);

// File reading functions
std::ifstream::pos_type filesize(std::string filename);
int get_file_pos(int * accum, int query, int & res);

// Getting one features from the binary files
void get_onefeatcomp(int query_ids, size_t read_size, int* accum, vector<ifstream*>& read_in_compfeatures, vector<ifstream*>& read_in_compidx, char* feature_cp);
void get_onefeat(int query_ids, size_t read_size, int* accum, vector<ifstream*>& read_in_features, char* feature_cp);

// Template functions have to be declared in header
template<class ty>
// L2 normalization of features
void normalize(ty *X, size_t dim)
{
    ty sum = 0;
    for (int i=0;i<dim;i++)
    {
        sum +=X[i]*X[i];
    }
    sum = sqrt(sum);
    ty n = 1 / sum;
    for (int i=0;i<dim;i++)
    {
        X[i] *= n;
    }
}