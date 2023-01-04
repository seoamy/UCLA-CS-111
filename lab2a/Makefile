build:
	gcc -Wall -Wextra -g -o lab2_add -lpthread lab2_add.c
	gcc -Wall -Wextra -g -o lab2_list -lpthread lab2_list.c SortedList.c

tests: build
	rm -rf lab2_list.csv
	rm -rf lab2_add.csv
	chmod +x add_tests.sh
	chmod +x list_tests.sh
	-./add_tests.sh
	-./list_tests.sh

graphs: tests
	chmod +x lab2_add.gp
	chmod +x lab2_list.gp
	gnuplot ./lab2_add.gp
	gnuplot ./lab2_list.gp

dist: graphs
	tar -czvf lab2a-505328863.tar.gz lab2_add.c lab2_list.c README Makefile add_tests.sh list_tests.sh lab2_add.csv lab2_list.csv SortedList.h SortedList.c lab2_add.gp lab2_list.gp lab2_add-1.png lab2_add-2.png lab2_add-3.png lab2_add-4.png lab2_add-5.png lab2_list-1.png lab2_list-2.png lab2_list-3.png lab2_list-4.png

clean:
	rm -rf lab2_add lab2_list lab2a-505328863.tar.gz *.o