# N = 1000; T = 200; eps = 1; leaf  
./bin/test_leaf_all_run 1 12345 bin265_PULocationID_ss1.txt 200 1 1000 2 1 0.001 &./bin/test_leaf_all_run 2 12345 bin265_PULocationID_ss2.txt 200 1 1000 2 1 0.001 

# N = 1000; T = 200; eps = 1; tree_naive 
./bin/test_tree_all_run 1 12345 bin265_PULocationID_ss1.txt 200 1 1000 0 0 0.001 &./bin/test_tree_all_run 2 12345 bin265_PULocationID_ss2.txt 200 1 1000 0 0 0.001 

# N = 1000; T = 200; eps = 1; tree_optimized 
./bin/test_tree_all_run 1 12345 bin265_PULocationID_ss1.txt 200 1 1000 2 0 0.001 &./bin/test_tree_all_run 2 12345 bin265_PULocationID_ss2.txt 200 1 1000 2 0 0.001 

# N = 1000; T = 600; eps = 1; tree_optimized 
./bin/test_tree_all_run 1 12345 bin265_PULocationID_ss1.txt 600 1 1000 2 0 0.001 &./bin/test_tree_all_run 2 12345 bin265_PULocationID_ss2.txt 600 1 1000 2 0 0.001 

#Tree approach 
echo "#T=200, eps= 2, N=1000"
./bin/test_tree_all_run 1 12345 bin265_PULocationID_ss1.txt 200 2 1000 2 0 0.001 &./bin/test_tree_all_run 2 12345 bin265_PULocationID_ss2.txt 200 2 1000 2 0 0.001 
#echo "#T=200, eps= 1, N=1000"
#./bin/test_tree_all_run 1 12345 bin265_PULocationID_ss1.txt 200 1 1000 2 0 0.001 &./bin/test_tree_all_run 2 12345 bin265_PULocationID_ss2.txt 200 1 1000 2 0 0.001 
echo "#T=200, eps= 0.5, N=1000"
./bin/test_tree_all_run 1 12345 bin265_PULocationID_ss1.txt 200 0.5 1000 2 0 0.001 &./bin/test_tree_all_run 2 12345 bin265_PULocationID_ss2.txt 200 0.5 1000 2 0 0.001 
echo "#T=200, eps= 0.25, N=1000"
./bin/test_tree_all_run 1 12345 bin265_PULocationID_ss1.txt 200 0.25 1000 2 0 0.001 &./bin/test_tree_all_run 2 12345 bin265_PULocationID_ss2.txt 200 0.25 1000 2 0 0.001 
#echo "#T=200, eps= 0.125, N=1000"
#./bin/test_tree_all_run 1 12345 bin265_PULocationID_ss1.txt 200 0.125 1000 2 0 0.001 &./bin/test_tree_all_run 2 12345 bin265_PULocationID_ss2.txt 200 0.125 1000 2 0 0.001 
echo "#T=200, eps= 1, N=1000, different p"
./bin/test_tree_all_run 1 12345 bin265_PULocationID_ss1.txt 200 1 1000 2 0 0.004 &./bin/test_tree_all_run 2 12345 bin265_PULocationID_ss2.txt 200 1 1000 2 0 0.004
./bin/test_tree_all_run 1 12345 bin265_PULocationID_ss1.txt 200 1 1000 2 0 0.007 &./bin/test_tree_all_run 2 12345 bin265_PULocationID_ss2.txt 200 1 1000 2 0 0.007
./bin/test_tree_all_run 1 12345 bin265_PULocationID_ss1.txt 200 1 1000 2 0 0.01 &./bin/test_tree_all_run 2 12345 bin265_PULocationID_ss2.txt 200 1 1000 2 0 0.01 


./bin/test_leaf_baseline_run 1 12345 bin265_PULocationID_ss1.txt 200 1 1000 2 1 0.001 &./bin/test_leaf_baseline_run 2 12345 bin265_PULocationID_ss2.txt 200 1 1000 2 1 0.001 
