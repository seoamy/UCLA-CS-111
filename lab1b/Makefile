# Name: Amy Seo
# Email: amyseo@g.ucla.edu
# ID: 505328863

default: lab1b-client.c lab1b-server.c
	gcc -g -lz -Wall -Wextra -o lab1b-server lab1b-server.c
	gcc -g -lz -Wall -Wextra -o lab1b-client lab1b-client.c

lab1b-client: lab1b-client.c
	gcc -g -lz -Wall -Wextra -o lab1b-client lab1b-client.c

lab1b-server: lab1b-server.c
	gcc -g -lz -Wall -Wextra -o lab1b-server lab1b-server.c

dist: lab1b-client.c lab1b-server.c README Makefile
	tar -cvzf lab1b-505328863.tar.gz README lab1b-client.c lab1b-server.c Makefile

clean:
	rm -rf *.tar.gz *.o lab1b-client lab1b-server