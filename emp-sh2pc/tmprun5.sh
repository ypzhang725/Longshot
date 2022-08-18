start_time=$(date +%s)
./bin/test_leaf_all_run 1 12345 bin40_ss1.txt 200 0.1 1000 2 1 &./bin/test_leaf_all_run 2 12345 bin40_ss2.txt 200 0.1 1000 2 1
end_time1=$(date +%s)
elapsed1=$(( end_time1 - start_time ))
echo $elapsed1
./bin/test_tree_all_run 1 12345 bin40_ss1.txt 200 0.1 1000 2 0 &./bin/test_tree_all_run 2 12345/ bin40_ss2.txt 200 0.1 1000 2 0
end_time2=$(date +%s)
elapsed2=$(( end_time2 - start_time ))
echo $elapsed2
#./bin/test_leaf_baseline_run 1 12345 bin40_ss1.txt 10 1 1000 2 1 &./bin/test_leaf_baseline_run 2 12345 bin40_ss2.txt 10 1 1000 2 1
#end_time3=$(date +%s)
#elapsed3=$(( end_time3 - start_time ))
#echo $elapsed3
./bin/test_tree_all_run 1 12345 bin40_ss1.txt 200 0.1 1000 0 0 &./bin/test_tree_all_run 2 12345 bin40_ss2.txt 200 0.1 1000 0 0
end_time4=$(date +%s)
elapsed4=$(( end_time4 - start_time ))
echo $elapsed4
