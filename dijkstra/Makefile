main : array_based heap_based
	./heap_based mapDist1600.csv 4 1 300 1000
	./heap_based mapDist1600.csv 4 1 200 1300
	./heap_based mapDist1600.csv 1 1 100 200
	./heap_based mapDist10000.csv 4 1 300 1000
	./heap_based mapDist10000.csv 4 1 200 1300
	./heap_based mapDist10000.csv 1 1 100 200
	./heap_based mapDist15000.csv 4 1 300 1000
	./heap_based mapDist15000.csv 4 1 200 1300
	./heap_based mapDist15000.csv 1 1 100 200
	echo "---------------------------------"
	./array_based mapDist1600.csv 4 1 300 1000
	./array_based mapDist1600.csv 4 1 200 1300
	./array_based mapDist1600.csv 1 1 100 200
	./array_based mapDist10000.csv 4 1 300 1000
	./array_based mapDist10000.csv 4 1 200 1300
	./array_based mapDist10000.csv 1 1 100 200
	./array_based mapDist15000.csv 4 1 300 1000
	./array_based mapDist15000.csv 4 1 200 1300
	./array_based mapDist15000.csv 1 1 100 200
	
array_based : array_based.cpp
	g++ ./array_based.cpp -o array_based -O2 -fopenmp

heap_based : heap_based.cpp
	g++ ./heap_based.cpp -o heap_based -O2 -fopenmp
