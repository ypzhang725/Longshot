#Tree approach 
echo "#T=200, eps= 2, N=1000"
./bin/test_tree_all_run 1 12345 bin40_ss1.txt 200 2 1000 2 0 &./bin/test_tree_all_run 2 12345 bin40_ss2.txt 200 2 1000 2 0
#echo "#T=200, eps= 1, N=1000"
#./bin/test_tree_all_run 1 12345 bin40_ss1.txt 200 1 1000 2 0 &./bin/test_tree_all_run 2 12345 bin40_ss2.txt 200 1 1000 2 0
echo "#T=200, eps= 0.5, N=1000"
./bin/test_tree_all_run 1 12345 bin40_ss1.txt 200 0.5 1000 2 0 &./bin/test_tree_all_run 2 12345 bin40_ss2.txt 200 0.5 1000 2 0
echo "#T=200, eps= 0.25, N=1000"
./bin/test_tree_all_run 1 12345 bin40_ss1.txt 200 0.25 1000 2 0 &./bin/test_tree_all_run 2 12345 bin40_ss2.txt 200 0.25 1000 2 0
echo "#T=200, eps= 0.125, N=1000"
./bin/test_tree_all_run 1 12345 bin40_ss1.txt 200 0.125 1000 2 0 &./bin/test_tree_all_run 2 12345 bin40_ss2.txt 200 0.125 1000 2 0

#Leaf approach 
#echo "#T=200, eps= 2, N=1000"
#./bin/test_leaf_all_run 1 12345 bin40_ss1.txt 200 2 1000 2 1 &./bin/test_leaf_all_run 2 12345 bin40_ss2.txt 200 2 1000 2 1
#echo "#T=200, eps= 1, N=1000"
#./bin/test_leaf_all_run 1 12345 bin40_ss1.txt 200 1 1000 2 1 &./bin/test_leaf_all_run 2 12345 bin40_ss2.txt 200 1 1000 2 1
#echo "#T=200, eps= 0.5, N=1000"
#./bin/test_leaf_all_run 1 12345 bin40_ss1.txt 200 0.5 1000 2 1 &./bin/test_leaf_all_run 2 12345 bin40_ss2.txt 200 0.5 1000 2 1
#echo "#T=200, eps= 0.25, N=1000"
#./bin/test_leaf_all_run 1 12345 bin40_ss1.txt 200 0.25 1000 2 1 &./bin/test_leaf_all_run 2 12345 bin40_ss2.txt 200 0.25 1000 2 1
#echo "#T=200, eps= 0.125, N=1000"
#./bin/test_leaf_all_run 1 12345 bin40_ss1.txt 200 0.125 1000 2 1 &./bin/test_leaf_all_run 2 12345 bin40_ss2.txt 200 0.125 1000 2 1

#Tree approach 
#echo "#T=200, eps= 2, N=1000"
#./bin/test_tree_all_run 1 12345 bin40_ss1.txt 600 2 1000 0 0 &./bin/test_tree_all_run 2 12345 bin40_ss2.txt 600 2 1000 0 0
#echo "#T=200, eps= 1, N=1000"
#./bin/test_tree_all_run 1 12345 bin40_ss1.txt 600 1 1000 0 0 &./bin/test_tree_all_run 2 12345 bin40_ss2.txt 600 1 1000 0 0
#echo "#T=200, eps= 0.5, N=1000"
#./bin/test_tree_all_run 1 12345 bin40_ss1.txt 600 0.5 1000 0 0 &./bin/test_tree_all_run 2 12345 bin40_ss2.txt 600 0.5 1000 0 0
#echo "#T=200, eps= 0.25, N=1000"
#./bin/test_tree_all_run 1 12345 bin40_ss1.txt 600 0.25 1000 0 0 &./bin/test_tree_all_run 2 12345 bin40_ss2.txt 600 0.25 1000 0 0
#echo "#T=200, eps= 0.125, N=1000"
#./bin/test_tree_all_run 1 12345 bin40_ss1.txt 600 0.125 1000 0 0 &./bin/test_tree_all_run 2 12345 bin40_ss2.txt 600 0.125 1000 0 0
