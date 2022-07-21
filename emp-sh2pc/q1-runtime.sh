for i in 1 2 3 4 5
    ./bin/test_leaf_baseline_run 1 12345 bin40_ss1.txt 10 1 1000 2 1 &./bin/test_leaf_baseline_run 2 12345 bin40_ss2.txt 10 1 1000 2 1
    ./bin/test_leaf_all_run 1 12345 bin40_ss1.txt 10 1 1000 2 1 &./bin/test_leaf_all_run 2 12345 bin40_ss1.txt 10 1 1000 2 1
    ./bin/test_tree_all_run 1 12345 bin40_ss1.txt 10 1 1000 2 0 &./bin/test_tree_all_run 2 12345 bin40_ss2.txt 10 1 1000 2 0
python3 pyRun_all.py
python3 pyRun_baseline.py