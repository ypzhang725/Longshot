# CMake generated Testfile for 
# Source directory: /Users/zhangyanping/Desktop/EncryptDB/emp_code/emp-ot/test
# Build directory: /Users/zhangyanping/Desktop/EncryptDB/emp_code/emp-ot/test
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(ot "./run" "/Users/zhangyanping/Desktop/EncryptDB/emp_code/emp-ot/bin/test_ot")
set_tests_properties(ot PROPERTIES  WORKING_DIRECTORY "/Users/zhangyanping/Desktop/EncryptDB/emp_code/emp-ot/" _BACKTRACE_TRIPLES "/Users/zhangyanping/Desktop/EncryptDB/emp_code/emp-ot/test/CMakeLists.txt;14;add_test;/Users/zhangyanping/Desktop/EncryptDB/emp_code/emp-ot/test/CMakeLists.txt;19;add_test_case_with_run;/Users/zhangyanping/Desktop/EncryptDB/emp_code/emp-ot/test/CMakeLists.txt;0;")
add_test(ferret "./run" "/Users/zhangyanping/Desktop/EncryptDB/emp_code/emp-ot/bin/test_ferret")
set_tests_properties(ferret PROPERTIES  WORKING_DIRECTORY "/Users/zhangyanping/Desktop/EncryptDB/emp_code/emp-ot/" _BACKTRACE_TRIPLES "/Users/zhangyanping/Desktop/EncryptDB/emp_code/emp-ot/test/CMakeLists.txt;14;add_test;/Users/zhangyanping/Desktop/EncryptDB/emp_code/emp-ot/test/CMakeLists.txt;20;add_test_case_with_run;/Users/zhangyanping/Desktop/EncryptDB/emp_code/emp-ot/test/CMakeLists.txt;0;")
add_test(bench_lpn "/Users/zhangyanping/Desktop/EncryptDB/emp_code/emp-ot/bin/test_bench_lpn")
set_tests_properties(bench_lpn PROPERTIES  WORKING_DIRECTORY "/Users/zhangyanping/Desktop/EncryptDB/emp_code/emp-ot/" _BACKTRACE_TRIPLES "/Users/zhangyanping/Desktop/EncryptDB/emp_code/emp-ot/test/CMakeLists.txt;9;add_test;/Users/zhangyanping/Desktop/EncryptDB/emp_code/emp-ot/test/CMakeLists.txt;21;add_test_case;/Users/zhangyanping/Desktop/EncryptDB/emp_code/emp-ot/test/CMakeLists.txt;0;")
