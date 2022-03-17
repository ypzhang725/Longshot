:'
for i in {1..6}
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

for i in {1..6}
do
   echo "Welcome $i times"
    #T:10; eps:10; N:100
    ./bin/test_leaf_all_run 1 12345 taxi_ss1.txt 10 10 100 1 &./bin/test_leaf_all_run 2 12345 taxi_ss2.txt 10 10 100 1
    ./bin/test_tree_all_run 1 12345 taxi_ss1.txt 10 10 100 2 1 &./bin/test_tree_all_run 2 12345 taxi_ss2.txt 10 10 100 2 1
    ./bin/test_tree_all_run 1 12345 taxi_ss1.txt 10 10 100 0 1 &./bin/test_tree_all_run 2 12345 taxi_ss2.txt 10 10 100 0 1

    #T:10; eps:1; N:100
    ./bin/test_leaf_all_run 1 12345 taxi_ss1.txt 10 1 100 1 &./bin/test_leaf_all_run 2 12345 taxi_ss2.txt 10 1 100 1
    ./bin/test_tree_all_run 1 12345 taxi_ss1.txt 10 1 100 2 1 &./bin/test_tree_all_run 2 12345 taxi_ss2.txt 10 1 100 2 1
    ./bin/test_tree_all_run 1 12345 taxi_ss1.txt 10 1 100 0 1 &./bin/test_tree_all_run 2 12345 taxi_ss2.txt 10 1 100 0 1

    #T:10; eps:0.1; N:100
    ./bin/test_leaf_all_run 1 12345 taxi_ss1.txt 10 0.1 100 1 &./bin/test_leaf_all_run 2 12345 taxi_ss2.txt 10 0.1 100 1
    ./bin/test_tree_all_run 1 12345 taxi_ss1.txt 10 0.1 100 2 1 &./bin/test_tree_all_run 2 12345 taxi_ss2.txt 10 0.1 100 2 1
    ./bin/test_tree_all_run 1 12345 taxi_ss1.txt 10 0.1 100 0 1 &./bin/test_tree_all_run 2 12345 taxi_ss2.txt 10 0.1 100 0 1
done
'
for i in {1..6}
do
   echo "Welcome $i times"
    #T:10; eps:10; N:10000
    ./bin/test_leaf_all_run 1 12345 taxi_ss1.txt 10 10 10000 1 &./bin/test_leaf_all_run 2 12345 taxi_ss2.txt 10 10 10000 1
    ./bin/test_tree_all_run 1 12345 taxi_ss1.txt 10 10 10000 2 1 &./bin/test_tree_all_run 2 12345 taxi_ss2.txt 10 10 10000 2 1
    ./bin/test_tree_all_run 1 12345 taxi_ss1.txt 10 10 10000 0 1 &./bin/test_tree_all_run 2 12345 taxi_ss2.txt 10 10 10000 0 1

    #T:10; eps:1; N:10000
    ./bin/test_leaf_all_run 1 12345 taxi_ss1.txt 10 1 10000 1 &./bin/test_leaf_all_run 2 12345 taxi_ss2.txt 10 1 10000 1
    ./bin/test_tree_all_run 1 12345 taxi_ss1.txt 10 1 10000 2 1 &./bin/test_tree_all_run 2 12345 taxi_ss2.txt 10 1 10000 2 1
    ./bin/test_tree_all_run 1 12345 taxi_ss1.txt 10 1 10000 0 1 &./bin/test_tree_all_run 2 12345 taxi_ss2.txt 10 1 10000 0 1
done

for i in {1..6}
do
   echo "Welcome $i times"
    #T:10; eps:0.1; N:10000
    ./bin/test_leaf_all_run 1 12345 taxi_ss1.txt 10 0.1 10000 1 &./bin/test_leaf_all_run 2 12345 taxi_ss2.txt 10 0.1 10000 1
    ./bin/test_tree_all_run 1 12345 taxi_ss1.txt 10 0.1 10000 2 1 &./bin/test_tree_all_run 2 12345 taxi_ss2.txt 10 0.1 10000 2 1
    ./bin/test_tree_all_run 1 12345 taxi_ss1.txt 10 0.1 10000 0 1 &./bin/test_tree_all_run 2 12345 taxi_ss2.txt 10 0.1 10000 0 1
done
