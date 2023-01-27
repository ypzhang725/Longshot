1. git clone https://github.com/ypzhang725/Longshot.git

2. cmake . &make  under emp-ot, emp-sh2pc, emp-tool

3. mkdir resultsEMPTWO under emp-sh2pc folder

4. change ip "127.0.0.1" to the ip of another machine in files: emp-sh2pc/test/leaf_all_run.cpp, emp-sh2pc/test/leaf_baseline_run.cpp, emp-sh2pc/test/tree_all_run.cpp

5. run code: ./vldb.sh

6. make plot using  EMP4.ipynb in folder resultsEMPTWO

