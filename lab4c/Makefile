# NAME: Amy Seo
# EMAIL: amyseo@g.ucla.edu
# ID: 505328863
NAME := $(shell uname -a)

default:
ifeq (armv7l,$(findstring armv7l,$(NAME)))
	gcc -Wall -Wextra -g -lmraa -lm -o lab4c_tcp lab4c_tcp.c
	gcc -Wall -Wextra -g -lmraa -lm -lcrypto -o lab4c_tls lab4c_tls.c
else
	gcc -Wall -Wextra -g -DDUMMY -lm -o lab4c_tcp lab4c_tcp.c
	gcc -Wall -Wextra -g -DDUMMY -lm -lssl -lcrypto -o lab4c_tls lab4c_tls.c
endif

clean:
	rm -rf lab4c_tcp lab4c_tls *.tar.gz DUMMY *.dYSM

dist:
	tar -czvf lab4c-505328863.tar.gz lab4c_tcp.c lab4c_tls.c Makefile README
