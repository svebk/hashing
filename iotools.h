#include <fstream>
#include <math.h>

// Compression functions
int compress_onefeat(char * in, char * comp, int fsize);
int decompress_onefeat(char * in, char * comp, int compsize, int fsize);

// File reading functions
std::ifstream::pos_type filesize(std::string filename);
int get_file_pos(int * accum, int query, int & res);


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