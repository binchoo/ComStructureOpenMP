main : max_test assign_test
	./max_test 10 4
	./max_test 100 4
	./max_test 1000 4
	./max_test 10000 4
	./assign_test 1000 4
	./assign_test 10000 4
	./assign_test 100000 4
	./assign_test 1000000 4
	./assign_test 10000000 4
	./assign_test 100000000 4
	./assign_test 214748364 8


max_test : max_test.cpp
	g++ ./max_test.cpp -o max_test -O2 -fopenmp
	
assign_test :
	g++ ./assign_test.cpp -o assign_test -O2 -fopenmp
