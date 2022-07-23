./bin/test_leaf_baseline_run 1 12345 bin40_ss1.txt 10 1 10 2 1 &./bin/test_leaf_baseline_run 2 12345 bin40_ss2.txt 10 1 10 2 1
./bin/test_leaf_all_run 1 12345 bin40_ss1.txt 10 1 10 2 1 &./bin/test_leaf_all_run 2 12345 bin40_ss1.txt 10 1 10 2 1
./bin/test_tree_all_run 1 12345 bin40_ss1.txt 10 1 10 2 0 &./bin/test_tree_all_run 2 12345 bin40_ss2.txt 10 1 10 2 0
./bin/test_leaf_baseline_run 1 12345 bin40_ss1.txt 10 1 10 2 1 &./bin/test_leaf_baseline_run 2 12345 bin40_ss2.txt 10 1 10 2 1
./bin/test_leaf_all_run 1 12345 bin40_ss1.txt 10 1 10 2 1 &./bin/test_leaf_all_run 2 12345 bin40_ss1.txt 10 1 10 2 1
./bin/test_tree_all_run 1 12345 bin40_ss1.txt 10 1 10 2 0 &./bin/test_tree_all_run 2 12345 bin40_ss2.txt 10 1 10 2 0
python3 pyRun_baseline.py
python3 pyRun_all.py
