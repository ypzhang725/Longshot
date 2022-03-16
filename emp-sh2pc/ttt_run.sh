for i in 1 2 3
do
   echo "Welcome $i times"
    #T:100; eps:10; N:100
    ./bin/test_leaf_all_run 1 12345 taxi_ss1.txt 100 10 100 1 &./bin/test_leaf_all_run 2 12345 taxi_ss2.txt 100 10 100 1
    ./bin/test_tree_all_run 1 12345 taxi_ss1.txt 100 10 100 2 1 &./bin/test_tree_all_run 2 12345 taxi_ss2.txt 100 10 100 2 1
    ./bin/test_tree_all_run 1 12345 taxi_ss1.txt 100 10 100 0 1 &./bin/test_tree_all_run 2 12345 taxi_ss2.txt 100 10 100 0 1

    #T:100; eps:1; N:100
    ./bin/test_leaf_all_run 1 12345 taxi_ss1.txt 100 1 100 1 &./bin/test_leaf_all_run 2 12345 taxi_ss2.txt 100 1 100 1
    ./bin/test_tree_all_run 1 12345 taxi_ss1.txt 100 1 100 2 1 &./bin/test_tree_all_run 2 12345 taxi_ss2.txt 100 1 100 2 1
    ./bin/test_tree_all_run 1 12345 taxi_ss1.txt 100 1 100 0 1 &./bin/test_tree_all_run 2 12345 taxi_ss2.txt 100 1 100 0 1

    #T:100; eps:0.1; N:100
    ./bin/test_leaf_all_run 1 12345 taxi_ss1.txt 100 0.1 100 1 &./bin/test_leaf_all_run 2 12345 taxi_ss2.txt 100 0.1 100 1
    ./bin/test_tree_all_run 1 12345 taxi_ss1.txt 100 0.1 100 2 1 &./bin/test_tree_all_run 2 12345 taxi_ss2.txt 100 0.1 100 2 1
    ./bin/test_tree_all_run 1 12345 taxi_ss1.txt 100 0.1 100 0 1 &./bin/test_tree_all_run 2 12345 taxi_ss2.txt 100 0.1 100 0 1
done

for i in 1 2 3
do
   echo "Welcome $i times"
    #T:10000; eps:10; N:100
    ./bin/test_leaf_all_run 1 12345 taxi_ss1.txt 10000 10 100 1 &./bin/test_leaf_all_run 2 12345 taxi_ss2.txt 10000 10 100 1
    ./bin/test_tree_all_run 1 12345 taxi_ss1.txt 10000 10 100 2 1 &./bin/test_tree_all_run 2 12345 taxi_ss2.txt 10000 10 100 2 1
    ./bin/test_tree_all_run 1 12345 taxi_ss1.txt 10000 10 100 0 1 &./bin/test_tree_all_run 2 12345 taxi_ss2.txt 10000 10 100 0 1

    #T:10000; eps:1; N:100
    ./bin/test_leaf_all_run 1 12345 taxi_ss1.txt 10000 1 100 1 &./bin/test_leaf_all_run 2 12345 taxi_ss2.txt 10000 1 100 1
    ./bin/test_tree_all_run 1 12345 taxi_ss1.txt 10000 1 100 2 1 &./bin/test_tree_all_run 2 12345 taxi_ss2.txt 10000 1 100 2 1
    ./bin/test_tree_all_run 1 12345 taxi_ss1.txt 10000 1 100 0 1 &./bin/test_tree_all_run 2 12345 taxi_ss2.txt 10000 1 100 0 1

    #T:10000; eps:0.1; N:100
    ./bin/test_leaf_all_run 1 12345 taxi_ss1.txt 10000 0.1 100 1 &./bin/test_leaf_all_run 2 12345 taxi_ss2.txt 10000 0.1 100 1
    ./bin/test_tree_all_run 1 12345 taxi_ss1.txt 10000 0.1 100 2 1 &./bin/test_tree_all_run 2 12345 taxi_ss2.txt 10000 0.1 100 2 1
    ./bin/test_tree_all_run 1 12345 taxi_ss1.txt 10000 0.1 100 0 1 &./bin/test_tree_all_run 2 12345 taxi_ss2.txt 10000 0.1 100 0 1
done
