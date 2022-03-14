#four input: datasetName; T; eps; N; outIndex
# simple dataset
#./bin/test_leaf_all_run 1 12345 ss_1.txt 8 1 10 &./bin/test_leaf_all_run 2 12345 ss_2.txt 8 1 10
# NYCTaxi dataset
./bin/test_leaf_all_run 1 12345 taxi_ss1.txt 10 10 1000 1 &./bin/test_leaf_all_run 2 12345 taxi_ss2.txt 10 10 1000 1