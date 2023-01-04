# Name: Amy Seo
# Email: amyseo@g.ucla.edu
# ID: 505328863

default: lab0.c
	gcc -g -Wall -Wextra -o lab0 lab0.c

dist: lab0.c Makefile README backtrace.png breakpoint.png smoke_tests.sh
	tar -cvzf lab0-505328863.tar.gz README lab0.c Makefile backtrace.png breakpoint.png smoke_tests.sh

check:
	./smoke_tests.sh

clean:
	rm -f *.o *.tar.gz lab0

