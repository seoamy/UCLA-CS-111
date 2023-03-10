 # test list
	# mean cost per operation vs number of operations
	./lab2_list --threads=1 --iterations=10 >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=1 --iterations=100 >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=1 --iterations=1000 >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=1 --iterations=10000 >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=1 --iterations=20000 >> lab2_list.csv 2> /dev/null
	# no yielding conflicts
	./lab2_list --threads=2 --iterations=1 >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=4 --iterations=1 >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=8 --iterations=1 >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=12 --iterations=1 >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=2 --iterations=10 >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=4 --iterations=10 >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=8 --iterations=10 >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=12 --iterations=10 >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=2 --iterations=100 >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=4 --iterations=100 >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=8 --iterations=100 >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=12 --iterations=100 >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=2 --iterations=1000 >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=4 --iterations=1000 >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=8 --iterations=1000 >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=12 --iterations=1000 >> lab2_list.csv 2> /dev/null
	# insert yield conflicts
	./lab2_list --threads=2 --iterations=1 --yield=i >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=2 --iterations=2 --yield=i >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=2 --iterations=4 --yield=i >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=2 --iterations=8 --yield=i >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=2 --iterations=16 --yield=i >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=2 --iterations=32 --yield=i >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=4 --iterations=1 --yield=i >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=4 --iterations=2 --yield=i >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=4 --iterations=4 --yield=i >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=4 --iterations=8 --yield=i >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=4 --iterations=16 --yield=i >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=4 --iterations=32 --yield=i >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=8 --iterations=1 --yield=i >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=8 --iterations=2 --yield=i >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=8 --iterations=4 --yield=i >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=8 --iterations=8 --yield=i >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=8 --iterations=16 --yield=i >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=8 --iterations=32 --yield=i >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=12 --iterations=1 --yield=i >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=12 --iterations=2 --yield=i >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=12 --iterations=4 --yield=i >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=12 --iterations=8 --yield=i >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=12 --iterations=16 --yield=i >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=12 --iterations=32 --yield=i >> lab2_list.csv 2> /dev/null
	# delete yield conflicts
	./lab2_list --threads=2 --iterations=1 --yield=d >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=2 --iterations=2 --yield=d >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=2 --iterations=4 --yield=d >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=2 --iterations=8 --yield=d >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=2 --iterations=16 --yield=d >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=2 --iterations=32 --yield=d >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=4 --iterations=1 --yield=d >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=4 --iterations=2 --yield=d >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=4 --iterations=4 --yield=d >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=4 --iterations=8 --yield=d >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=4 --iterations=16 --yield=d >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=4 --iterations=32 --yield=d >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=8 --iterations=1 --yield=d >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=8 --iterations=2 --yield=d >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=8 --iterations=4 --yield=d >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=8 --iterations=8 --yield=d >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=8 --iterations=16 --yield=d >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=8 --iterations=32 --yield=d >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=12 --iterations=1 --yield=d >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=12 --iterations=2 --yield=d >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=12 --iterations=4 --yield=d >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=12 --iterations=8 --yield=d >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=12 --iterations=16 --yield=d >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=12 --iterations=32 --yield=d >> lab2_list.csv 2> /dev/null
	# insert & lookup yield conflicts
	./lab2_list --threads=2 --iterations=1 --yield=il >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=2 --iterations=2 --yield=il >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=2 --iterations=4 --yield=il >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=2 --iterations=8 --yield=il >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=2 --iterations=16 --yield=il >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=2 --iterations=32 --yield=il >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=4 --iterations=1 --yield=il >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=4 --iterations=2 --yield=il >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=4 --iterations=4 --yield=il >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=4 --iterations=8 --yield=il >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=4 --iterations=16 --yield=il >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=4 --iterations=32 --yield=il >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=8 --iterations=1 --yield=il >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=8 --iterations=2 --yield=il >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=8 --iterations=4 --yield=il >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=8 --iterations=8 --yield=il >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=8 --iterations=16 --yield=il >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=8 --iterations=32 --yield=il >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=12 --iterations=1 --yield=il >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=12 --iterations=2 --yield=il >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=12 --iterations=4 --yield=il >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=12 --iterations=8 --yield=il >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=12 --iterations=16 --yield=il >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=12 --iterations=32 --yield=il >> lab2_list.csv 2> /dev/null
	# delete & lookup yield conflicts
	./lab2_list --threads=2 --iterations=1 --yield=dl >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=2 --iterations=2 --yield=dl >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=2 --iterations=4 --yield=dl >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=2 --iterations=8 --yield=dl >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=2 --iterations=16 --yield=dl >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=2 --iterations=32 --yield=dl >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=4 --iterations=1 --yield=dl >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=4 --iterations=2 --yield=dl >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=4 --iterations=4 --yield=dl >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=4 --iterations=8 --yield=dl >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=4 --iterations=16 --yield=dl >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=4 --iterations=32 --yield=dl >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=8 --iterations=1 --yield=dl >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=8 --iterations=2 --yield=dl >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=8 --iterations=4 --yield=dl >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=8 --iterations=8 --yield=dl >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=8 --iterations=16 --yield=dl >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=8 --iterations=32 --yield=dl >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=12 --iterations=1 --yield=dl >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=12 --iterations=2 --yield=dl >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=12 --iterations=4 --yield=dl >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=12 --iterations=8 --yield=dl >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=12 --iterations=16 --yield=dl >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=12 --iterations=32 --yield=dl >> lab2_list.csv 2> /dev/null
	# spin-lock & mutex
	./lab2_list --threads=12 --iterations=32 --yield=i --sync=m >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=12 --iterations=32 --yield=d --sync=m >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=12 --iterations=32 --yield=il --sync=m >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=12 --iterations=32 --yield=dl --sync=m >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=12 --iterations=32 --yield=i --sync=s >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=12 --iterations=32 --yield=d --sync=s >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=12 --iterations=32 --yield=il --sync=s >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=12 --iterations=32 --yield=dl --sync=s >> lab2_list.csv 2> /dev/null
	# per operation times vs number of threads
	./lab2_list --threads=1 --iterations=1000 --sync=m >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=2 --iterations=1000 --sync=m >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=4 --iterations=1000 --sync=m >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=8 --iterations=1000 --sync=m >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=12 --iterations=1000 --sync=m >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=16 --iterations=1000 --sync=m >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=24 --iterations=1000 --sync=m >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=1 --iterations=1000 --sync=s >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=2 --iterations=1000 --sync=s >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=4 --iterations=1000 --sync=s >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=8 --iterations=1000 --sync=s >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=12 --iterations=1000 --sync=s >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=16 --iterations=1000 --sync=s >> lab2_list.csv 2> /dev/null
	./lab2_list --threads=24 --iterations=1000 --sync=s >> lab2_list.csv 2> /dev/null