build:
	gcc -Wall -Wextra -g -o lab2_list -lpthread lab2_list.c SortedList.c

tests: build
	rm -rf lab2b_list.csv
	chmod +x list_tests.sh
	-./list_tests.sh

graphs: tests
	chmod +x lab2b.gp
	-gnuplot ./lab2b.gp

profile:
	LD_PRELOAD=/usr/lib64/libprofiler.so CPUPROFILE=./raw.gperf ./lab2_list --threads=12 --iterations=1000 --sync=s
	pprof --text ./lab2_list ./raw.gperf > profile.out
	pprof --list=thread_worker ./lab2_list ./raw.gperf >> profile.out

dist: graphs
	tar -czvf lab2b-505328863.tar.gz lab2_list.c README Makefile lab2b_list.csv lab2b.gp SortedList.c SortedList.h profile.out list_tests.sh lab2b_1.png lab2b_2.png lab2b_3.png lab2b_4.png lab2b_5.png

clean:
	rm -rf lab2_list lab2b-505328863.tar.gz *.o