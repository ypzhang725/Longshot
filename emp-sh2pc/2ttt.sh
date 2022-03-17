for i in {1..3}
do
   echo "Welcome $i times"
    #T:10; eps:10; N:1000
    ./bin/test_leaf_all_run 1 12345 taxi_ss1.txt 10 10 1000 1 &./bin/test_leaf_all_run 2 12345 taxi_ss2.txt 10 10 1000 1
    ./bin/test_tree_all_run 1 12345 taxi_ss1.txt 10 10 1000 2 1 &./bin/test_tree_all_run 2 12345 taxi_ss2.txt 10 10 1000 2 1
    ./bin/test_tree_all_run 1 12345 taxi_ss1.txt 10 10 1000 0 1 &./bin/test_tree_all_run 2 12345 taxi_ss2.txt 10 10 1000 0 1

    #T:10; eps:1; N:1000
    ./bin/test_leaf_all_run 1 12345 taxi_ss1.txt 10 1 1000 1 &./bin/test_leaf_all_run 2 12345 taxi_ss2.txt 10 1 1000 1
    ./bin/test_tree_all_run 1 12345 taxi_ss1.txt 10 1 1000 2 1 &./bin/test_tree_all_run 2 12345 taxi_ss2.txt 10 1 1000 2 1
    ./bin/test_tree_all_run 1 12345 taxi_ss1.txt 10 1 1000 0 1 &./bin/test_tree_all_run 2 12345 taxi_ss2.txt 10 1 1000 0 1

    #T:10; eps:0.1; N:1000
    ./bin/test_leaf_all_run 1 12345 taxi_ss1.txt 10 0.1 1000 1 &./bin/test_leaf_all_run 2 12345 taxi_ss2.txt 10 0.1 1000 1
    ./bin/test_tree_all_run 1 12345 taxi_ss1.txt 10 0.1 1000 2 1 &./bin/test_tree_all_run 2 12345 taxi_ss2.txt 10 0.1 1000 2 1
    ./bin/test_tree_all_run 1 12345 taxi_ss1.txt 10 0.1 1000 0 1 &./bin/test_tree_all_run 2 12345 taxi_ss2.txt 10 0.1 1000 0 1
done
