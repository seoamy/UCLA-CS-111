# NAME: Amy Seo
# EMAIL: amyseo@g.ucla.edu
# ID: 505328863
NAME := $(shell uname -a)

default:
ifeq (armv7l,$(findstring armv7l,$(NAME)))
	gcc -Wall -Wextra -g -lmraa -lm -o lab4b lab4b.c
else
	gcc -Wall -Wextra -g -DDUMMY -lm -o lab4b lab4b.c
endif

check: default
	chmod +x smoke_tests.sh
	./smoke_tests.sh

clean:
	rm -rf lab4b *.tar.gz DUMMY *.dYSM

dist:
	tar -czvf lab4b-505328863.tar.gz lab4b.c Makefile README smoke_tests.sh
