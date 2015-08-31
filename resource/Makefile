default :
	make clear_lib
	make queue_q
	make thread_t
	make log_l
	make ini_i
	make pool_p
	make data_d
	make socket_s
	make main_m

queue_q : queue.h queue.c
	rm -f queue
	gcc -c queue.c
	gcc -Wall queue.o -o queue
	#clear
	./queue

thread_t : thread.h thread.c
	rm -f thread
	gcc -c thread.c
	gcc -Wall thread.o -o thread
#	clear
	./thread

log_l : log.h log.c
	rm -f log
	gcc -c log.c
	gcc -Wall log.o -o log
	clear
	./log

ini_i : ini.h ini.c
	rm -f ini
	gcc -c ini.c
	gcc -Wall ini.o -o ini
	clear
	./ini

pool_p : pool.h pool.c thread.h thread.c queue.h queue.c
	rm -f pool 
	gcc -c pool.c thread.c queue.c
	gcc -Wall pool.o thread.o queue.o -o pool
#	clear 
	./pool

data_d : data.h data.c
	rm -f data 
	gcc -c data.c
	gcc -Wall data.o -o data
#	clear 
	./data

socket_s : pool.h pool.c thread.h thread.c queue.h queue.c socket.h socket.c data.h data.c
	rm -f socket
	gcc -c pool.c thread.c queue.c socket.c data.c
	gcc -Wall pool.o thread.o queue.o socket.o data.o -o socket
#	clear 
	./socket

main_m : pool.h pool.c thread.h thread.c queue.h queue.c socket.h socket.c data.h data.c main.c
	make clear_lib
	gcc -c pool.h pool.c thread.h thread.c queue.h queue.c socket.h socket.c data.h data.c main.c
	gcc -Wall pool.o thread.o queue.o socket.o data.o main.o -o main
#	clear 
	./main

clear_lib :
	rm -f thread.o queue.o log.o ini.o pool.o socket.o data.o main.o
	rm -f thread queue log ini pool socket data main
	clear