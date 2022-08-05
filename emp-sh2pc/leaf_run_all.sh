#four input: datasetName; T; eps; N; sortOption(0: sort subRoot; 2; sortD); outIndex
# simple dataset for debug
#./bin/test_leaf_all_run 1 12345 ss_1.txt 8 10 10 2 1 &./bin/test_leaf_all_run 2 12345 ss_2.txt 8 10 10 2 1
# NYCTaxi dataset
#./bin/test_leaf_all_run 1 12345 taxi_ss1.txt 10 1 1000 2 1 &./bin/test_leaf_all_run 2 12345 taxi_ss2.txt 10 1 1000 2 1
./bin/test_leaf_all_run 1 12345 bin40_ss1.txt 100 1 1000 2 1 &./bin/test_leaf_all_run 2 12345 bin40_ss2.txt 100 1 1000 2 1
