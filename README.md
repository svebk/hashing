# hashing
hashing is released under BSD. Author: Tao Chen, generalmilk@gmail.com

1. Compling:
make

2. Before use, provide following 4 files:
W[_norm]_<hashing_bits>
mvec[_norm]_<hashing_bits>
itq[_norm]_<hashing_bits>
feature[_norm]
 
W, mvec and itq are the weight matrix, bias and ITQ hash bits of all features generated by ITQ hashing, feature is the raw binary file.of all features.
If the features are normalized, put [_norm] for each file. <hashing_bits> is hashing bit size.
Example filenames:
(1). W_norm_256, mvec_norm_256, itq_norm_256, feature_norm
(2). W_512, mvec_512, itq_512, feature

All files are raw binary:
W: <feature_dimension>*<hashing_bits> doubles
mvec: 1*<hashing_bits> doubles
itq: <feature_size>*<hashing_bits> bits
feature: <feature_size>*<feature_dimension> floats

feature_dimension is hardcoded (4096).

3. Usage: hashing feature_file_name [hashing_bits post_ranking_ratio nomarlize_features]
feature_file_name is the file name of query feautre(s). The format is the same with feature[_norm], can query multiple features at a time.
nomarlize_features can be 0 or 1, indicating whether the query feature(s) need to be normalized, default 1.

output: feature_file_name-sim_<post_ranking_ratio>.txt
Output file provides the ranked similarity query results. Each line is the query result for one feature, with 2*<feature_size>*<post_ranking_ratio> numbers.
The first <feature_size>*<post_ranking_ratio> are ranked index of similar features, the others are correponding distances.
If you have multiple query features, you will get multiple lines in output file.
