#four input: datasetName; T; eps; N; sortOption(0: sort subRoot; 2; sortD)
# simple dataset for debug
./bin/test_tree_all_run 1 12345 ss_1.txt 8 10 10 2 &./bin/test_tree_all_run 2 12345 ss_2.txt 8 10 10 2
# NYCTaxi dataset
#./bin/test_tree_all_run 1 12345 taxi_ss1.txt 8 10 1000 2 &./bin/test_tree_all_run 2 12345 taxi_ss2.txt 8 10 1000 2
