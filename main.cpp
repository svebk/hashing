#include "header.h"
#include <omp.h>
//#include <vl/generic.h>

#include <opencv2/opencv.hpp>

//#include <math.h>
#include <fstream>


using namespace std;
using namespace cv;

#define DEMO 1
#define INIT_FEAT 1

template<class ty>
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
int NumberOfSetBits(unsigned int i)
{
	i = i - ((i >> 1) & 0x55555555);
	i = (i & 0x33333333) + ((i >> 2) & 0x33333333);
	return (((i + (i >> 4)) & 0x0F0F0F0F) * 0x01010101) >> 24;
}
int count_bits(unsigned int n) {     
	unsigned int c; // c accumulates the total bits set in v
	for (c = 0; n; c++) 
		n &= n - 1; // clear the least significant bit set
	return c;
}
ifstream::pos_type filesize(string filename)
{
	ifstream in(filename, ios::ate | ios::binary);
	return in.tellg(); 
}

int countHammDist(unsigned int n, unsigned int m)
{
	int i=0;
	unsigned int count = 0 ;
	for(i=0; i<8; i++){
		if ((n&1) != (m&1)){
			count++;
		}
		n >>= 1;
		m >>= 1;

	}
	return count;
}
typedef std::pair<int,int> mypair;
typedef std::pair<float,int> mypairf;

bool comparator ( const mypair & l, const mypair & r)
{ return l.first < r.first; }
bool comparatorf ( const mypairf & l, const mypairf & r)
{ return l.first < r.first; }

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
	float runtimes[6] = {0.0f,0.0f,0.0f,0.0f,0.0f,0.0f};
	if (argc < 2){
		cout << "Usage: hashing feature_file_name [hashing_bits post_ranking_ratio nomarlize_features read_threshold]" << std::endl;

		return -1;
	}
	omp_set_num_threads(omp_get_max_threads());


	// hardcoded
	int feature_dim = 4096;
	float ratio = 0.001f;
	int bit_num = 256;
	int norm = true;
	if (argc>2)
		bit_num = atoi(argv[2]);
	if (argc>3)
		ratio = (float)atof(argv[3]);
	if (argc>4)
		norm = atoi(argv[4]);

	int read_thres = (int)(1.0f/ratio);
	if (argc>5)
		read_thres =  atoi(argv[5]);
	int int_num = bit_num/32;
	string bit_string = to_string((long long)bit_num);
	string str_norm = "";
	if (norm)
		str_norm = "norm_";
	string itq_name = "itq_" + str_norm + bit_string;
	string W_name = "W_" + str_norm + bit_string;
	string mvec_name = "mvec_" + str_norm + bit_string;

	//read in query
	int	query_num = (int)filesize(argv[1])/4/feature_dim;
	std::cout << "Hashing for " << query_num << " queries." << std::endl;
	ifstream read_in(argv[1],ios::in|ios::binary);
	if (!read_in.is_open())
	{
		std::cout << "Cannot load the query feature file!" << std::endl;
		return -1;
	}
	Mat query_mat(query_num,feature_dim,CV_32F);
	size_t read_size = sizeof(float)*feature_dim*query_num;
	read_in.read((char*)query_mat.data, read_size);
	std::cout << "Read " << read_size <<  " bytes for " << query_num << " queries." << std::endl;
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
	unsigned long long int data_num=0;

	if (INIT_FEAT) {
	data_nums.push_back((unsigned long long int)filesize(itq_name)*8/bit_num);
	data_num=data_nums[0];
	}
	for (int i=0;i<update_hash_files.size();i++)
	{
		data_nums.push_back((unsigned long long int)filesize(update_hash_files[i])*8/bit_num);
		if (INIT_FEAT) {
			data_num +=data_nums[i+1];
		} else {
			data_num +=data_nums[i];
			std::cout << "We have a " << data_nums[i] << " features in file " << update_hash_files[i] << std::endl;
		}
	}
	std::cout << "We have a total of " << data_num << " features." << std::endl;
	int top_feature=(int)ceil(data_num*ratio);
	std::cout << "We will get " << top_feature << " features." << std::endl;
	//std::cout << "Loading itq..." << std::endl;
	Mat itq(data_num,int_num,CV_32SC1);
	read_size=0;
	if (INIT_FEAT) {
        read_in.open(itq_name,ios::in|ios::binary);
        if (!read_in.is_open())
        {
                std::cout << "Cannot load the itq model!" << std::endl;
                return -1;
        }
	read_size = sizeof(int)*data_nums[0]*int_num;
	read_in.read((char*)itq.data, read_size);
	read_in.close();
	}
	char * read_pos = (char*)itq.data+ read_size;
	for (int i=0;i<update_hash_files.size();i++)
	{
		read_in.open(update_hash_files[i],ios::in|ios::binary);
		if (!read_in.is_open())
		{
			std::cout << "Cannot load the itq updates! File "<< update_hash_files[i] << std::endl;
			return -1;
		}
		if  (INIT_FEAT) {
			read_size = sizeof(int)*data_nums[i+1]*int_num;
		} else {
			read_size = sizeof(int)*data_nums[i]*int_num;
		}
		read_in.read(read_pos, read_size);
		read_in.close();
		read_pos +=read_size;
	} 

	read_in.open(W_name,ios::in|ios::binary);
	if (!read_in.is_open())
	{
		std::cout << "Cannot load the W model!" << std::endl;
		return -1;
	}
	Mat W(feature_dim,bit_num,CV_64F);
	read_size = sizeof(double)*feature_dim*bit_num;
	read_in.read((char*)W.data, read_size);
	read_in.close();

	read_in.open(mvec_name,ios::in|ios::binary);
	if (!read_in.is_open())
	{
		std::cout << "Cannot load the mvec model!" << std::endl;
		return -1;
	}
	Mat mvec(1,bit_num,CV_64F);
	read_size = sizeof(double)*bit_num;
	read_in.read((char*)mvec.data, read_size);
	read_in.close();

	//std::cout << "Loading features..." << std::endl;
	if (INIT_FEAT) {
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
	}
	Mat feature;
	if (query_num>read_thres)
	{
		feature.create(data_num,feature_dim,CV_32F);
		read_size=0;
		if (INIT_FEAT) {
		read_size = sizeof(float)*data_nums[0]*feature_dim;
		read_in.read((char*)feature.data, read_size);
		read_in.close();
		}
		read_pos = (char*)feature.data+ read_size;
		for (int i=0;i<update_feature_files.size();i++)
		{
			read_in.open(update_feature_files[i],ios::in|ios::binary);
			if (!read_in.is_open())
			{
				std::cout << "Cannot load the feature updates!" << std::endl;
				return -1;
			}
			if (INIT_FEAT) {
				read_size = sizeof(float)*data_nums[i+1]*feature_dim;
			} else {
				read_size = sizeof(float)*data_nums[i]*feature_dim;
			}
			read_in.read(read_pos, read_size);
			read_in.close();
			read_pos +=read_size;
		} 
	}
	else
	{
		
		feature.create(top_feature,feature_dim,CV_32F);
		read_size = sizeof(float)*feature_dim;
		if (INIT_FEAT) {
			read_in_features.push_back(&read_in);
		}
		int pos=0;
		for (int i=0;i<update_feature_files.size();i++)
		{
			read_in_features.push_back(new ifstream);
			if (INIT_FEAT) {
				pos=i+1;
			} else {
				pos=i;
			}
			read_in_features[pos]->open(update_feature_files[i],ios::in|ios::binary);
			if (!read_in_features[pos]->is_open())
			{
				std::cout << "Cannot load the feature updates!" << std::endl;
				return -1;
			}
		} 
	}
	//std::cout << "Loaded all features!" << std::endl;
	runtimes[0]=(float)(get_wall_time() - t[0]);

	//demo
	//int query_idx = 76;
	//float * query_feature = (float*)feature.data+feature_dim*query_idx;
	//unsigned int * query= (unsigned int*)itq.data+int_num*query_idx;

	//hashing init
	t[1]=get_wall_time();
	if (norm)
	{
		for  (int k=0;k<query_num;k++)
			normalize((float*)query_mat.data+k*feature_dim,feature_dim);
	}
	Mat query_mat_double;
	query_mat.convertTo(query_mat_double, CV_64F);
	mvec = repeat(mvec, query_num,1);
	Mat query_hash = query_mat_double*W-mvec;
	unsigned int * query_all = new unsigned int[int_num*query_num];
	for  (int k=0;k<query_num;k++)
	{
		for (int i=0;i<int_num;i++)
		{
			query_all[k*int_num+i] = 0;
			for (int j=0;j<32;j++)
				if (query_hash.at<double>(k,i*32+j)>0)
					query_all[k*int_num+i] += 1<<j;
		}
	}
	vector<mypair> hamming(data_num);
	vector<mypairf> postrank(top_feature);
	string outname = argv[1];
	outname.resize(outname.size()-4);
	string outname_sim = outname+"-sim.txt";
	ofstream outputfile;
	outputfile.open(outname_sim,ios::out);
	ofstream outputfile_hamming;
	if (DEMO==0) {
		string outname_hamming = outname+"-hamming.txt";
		outputfile_hamming.open(outname_hamming,ios::out);
	}
	unsigned int * query = query_all;
	float * query_feature = (float*)query_mat.data;
	runtimes[1]=(float)(get_wall_time() - t[1]);
	// Parallelize this for batch processing.
	for  (int k=0;k<query_num;k++)
	{
		std::cout <<  "Looking for similar images of query #" << k+1 << std::endl;
		//hashing
		unsigned int * hash_data= (unsigned int*)itq.data;
		t[1]=get_wall_time();
		for (int i=0;i<data_num;i++)
		{
			hamming[i] = mypair(0,i);
			for (int j=0;j<int_num;j++)
			{
				unsigned int xnor = query[j]^hash_data[j];
				hamming[i].first += NumberOfSetBits(xnor);
			}
			hash_data += int_num;
		}
		//cout << "what" <<hamming[2757278].first << std::endl;
		std::sort(hamming.begin(),hamming.end(),comparator);
		query += int_num;
		runtimes[2]+=(float)(get_wall_time() - t[1]);

		//read needed feature
		if (query_num<=read_thres)
		{
			t[1]=get_wall_time();
			char* feature_p = (char*)feature.data;
			//cout << "what" <<hamming[1].first <<" "<<hamming[1].second << std::endl;
			//cout << (unsigned int)(hamming[0].second)*4*feature_dim <<endl;
			int * accum = new int[data_nums.size()];
			accum[0]=data_nums[0];
			for (int i=1;i<data_nums.size();i++)
			{
				accum[i]=accum[i-1]+data_nums[i];
			}
			int i = 0;
			for (;i<top_feature;i++)
			{
				int new_pos,file_id;
				file_id= get_file_pos(accum,hamming[i].second,new_pos);
				read_in_features[file_id]->seekg((unsigned long long int)(new_pos)*4*feature_dim);
				//cout<<read_in.tellg()<<endl;
				read_in_features[file_id]->read(feature_p, read_size);
				feature_p +=read_size;
			}
			cout<<"Biggest hamming distance is: "<<hamming[i].first<<endl;
			delete[] accum;
			runtimes[0]+=(float)(get_wall_time() - t[1]);
		}


		//post ranking
		t[1]=get_wall_time();
		float* data_feature;
		if (norm)
		{
			for (int i=0;i<top_feature;i++)
			{
				postrank[i]= mypairf(1.0f,hamming[i].second);
				if (query_num>read_thres)
					data_feature = (float*)feature.data+feature_dim*postrank[i].second;
				else
					data_feature = (float*)feature.data+feature_dim*i;

				for (int j=0;j<feature_dim;j++)
				{
					postrank[i].first-=query_feature[j]*data_feature[j];
				}
			}
		}
		else
		{
			for (int i=0;i<top_feature;i++)
			{
				postrank[i]= mypairf(0.0f,hamming[i].second);
				if (query_num>read_thres)
					data_feature = (float*)feature.data+feature_dim*postrank[i].second;
				else
					data_feature = (float*)feature.data+feature_dim*i;

				for (int j=0;j<feature_dim;j++)
				{
					postrank[i].first+=pow(query_feature[j]-data_feature[j],2);
				}
				//postrank[i].first= sqrt(postrank[i].first);
			}
		}
		std::sort(postrank.begin(),postrank.end(),comparatorf);
		query_feature +=feature_dim;
		runtimes[3]+=(float)(get_wall_time() - t[1]);
		//cout << postrank[0].second << std::endl;
		//output
		t[1]=get_wall_time();
		for (int i=0;i<top_feature;i++) {
			outputfile << postrank[i].second << ' ';
			if (DEMO==0) {
				outputfile_hamming << postrank[i].second << ' ';
			}
		}
		for (int i=0;i<top_feature;i++) {
			outputfile << postrank[i].first << ' ';
			if (DEMO==0) {
				outputfile_hamming << hamming[i].first << ' ';
			}
		}
		outputfile << endl;
		if (DEMO==0) {
			outputfile_hamming << endl;
		}
		runtimes[4]+=(float)(get_wall_time() - t[1]);

	}
	delete[] query_all;
	outputfile.close();
	if (DEMO==0) {
		outputfile_hamming.close();
	}
	read_in.close();
	for (int i = 1; i<data_nums.size();i++)
	{
		read_in_features[i]->close();
		delete read_in_features[i];
	}

	cout << "loading (seconds): " << runtimes[0] << std::endl;
	cout << "hashing init (seconds): " << runtimes[1] << std::endl;
	cout << "hashing (seconds): " << runtimes[2] << std::endl;
	cout << "post ranking (seconds): " << runtimes[3] << std::endl;
	cout << "output (seconds): " << runtimes[4] << std::endl;
	cout << "total time (seconds): " << (float)(get_wall_time() - t[0]) << std::endl;
	return 0;
}

