#include "header.h"
#include <omp.h>
//#include <vl/generic.h>

#include <opencv2/opencv.hpp>

//#include <math.h>
#include <fstream>


using namespace std;
using namespace cv;


ifstream::pos_type filesize(string filename)
{
	ifstream in(filename, ios::ate | ios::binary);
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

int main(int argc, char** argv){
	double t[2]; // timing
	t[0] = get_wall_time(); // Start Time
	if (argc < 2){
		cout << "Usage: get_precomp_feats feature_ids_file_name feature_file_name [num_bits normalize_features]" << std::endl;

		return -1;
	}
	omp_set_num_threads(omp_get_max_threads());

	// hardcoded
	int feature_dim = 4096;
	int norm = true;
	// we just use hashing files to know number of features per update here.
	// so if multiple hashing are computed, use the smallest number of bits available.
	int bit_num = 256; 
	string ids_file(argv[1]);
	string out_file(argv[2]);
	if (argc>3)
		bit_num = atoi(argv[3]);
	if (argc>4)
		norm = atoi(argv[4]);

	int int_num = bit_num/32;
	string bit_string = to_string((long long)bit_num);
	string str_norm = "";
	if (norm)
		str_norm = "norm_";
	string itq_name = "itq_" + str_norm + bit_string;
	string W_name = "W_" + str_norm + bit_string;
	string mvec_name = "mvec_" + str_norm + bit_string;

	//read in query
	int	query_num = (int)filesize(argv[1])/sizeof(int);
	ifstream read_in(argv[1],ios::in|ios::binary);
	if (!read_in.is_open())
	{
		std::cout << "Cannot load the query feature ids file!" << std::endl;
		return -1;
	}
	// read query ids
	int* query_ids = new int[query_num];
	size_t read_size = sizeof(int)*query_num;
	read_in.read((char*)query_ids, read_size);
	read_in.close();

	//config update
	string line;
	vector<string> update_hash_files;
	vector<string> update_feature_files;
	string update_hash_prefix = "update/hash_bits/";
	string update_feature_prefix = "update/features/";
	string update_hash_suffix = "";
	string update_feature_suffix = "";
	if (norm)
	{
		update_hash_suffix = "_" + itq_name;
		update_feature_suffix = "_norm";
	}
	ifstream fu("update_list.txt",ios::in);
	if (!fu.is_open())
	{
		std::cout << "no update" << std::endl;
	}
	else
	{
		while (getline(fu, line)) {
			update_hash_files.push_back(update_hash_prefix+line+update_hash_suffix);
			update_feature_files.push_back(update_feature_prefix+line+update_feature_suffix);

		}
	}
	vector<ifstream*> read_in_features;
	// read in itq
	vector<unsigned long long int> data_nums;
	data_nums.push_back((unsigned long long int)filesize(itq_name)*8/bit_num);
	unsigned long long int data_num=data_nums[0];
	for (int i=0;i<update_hash_files.size();i++)
	{
		data_nums.push_back((unsigned long long int)filesize(update_hash_files[i])*8/bit_num);
		data_num +=data_nums[i+1];
	}

	//read in feature
	if (norm)
		read_in.open("feature_norm",ios::in|ios::binary);
	else
		read_in.open("feature",ios::in|ios::binary);
	if (!read_in.is_open())
	{
		std::cout << "Cannot load the feature file!" << std::endl;
		return -1;
	}
		
	float* feature = new float[query_num*feature_dim*sizeof(float)];
	read_size = sizeof(float)*feature_dim;
	read_in_features.push_back(&read_in);
	for (int i=0;i<update_feature_files.size();i++)
	{
		read_in_features.push_back(new ifstream);
		read_in_features[i+1]->open(update_feature_files[i],ios::in|ios::binary);
		if (!read_in_features[i+1]->is_open())
			{
				std::cout << "Cannot load the feature updates!" << std::endl;
				return -1;
			} 
	}

	char* feature_cp = (char*)feature.data;
	int * accum = new int[data_nums.size()];
	accum[0]=data_nums[0];
	for (int i=1;i<data_nums.size();i++)
	{
		accum[i]=accum[i-1]+data_nums[i];
	}
	
		
	for (int i=0;i<query_num;i++)
	{
		int new_pos,file_id;
		file_id= get_file_pos(accum,query_ids[i],new_pos);
		//read_in_features[file_id]->seekg((unsigned long long int)(new_pos)*sizeof(float)*feature_dim);
		read_in_features[file_id]->seekg((unsigned long long int)(new_pos)*4*feature_dim);
		//cout<<read_in.tellg()<<endl;
		read_in_features[file_id]->read(feature_cp, read_size);
		feature_cp +=read_size;
	}
	
	delete[] accum;
	delete[] query_ids;
	
	// properly close
	read_in.close();
	for (int i = 1; i<data_nums.size();i++)
	{
		read_in_features[i]->close();
		delete read_in_features[i];
	}

	// write out features to out_file
	ofstream output(out_file,ofstream::binary);
	float* feature_p = (float*)feature;
	for (int i = 0; i<query_num; i++) {
		output.write(feature_p,read_size);
		feature_p +=  read_size;
	}
	output.close();

	cout << "total time (seconds): " << (float)(get_wall_time() - t[0]) << std::endl;
	return 0;
}

