all:	sender receiver

sender: sender.c
	gcc -pthread -Wall $< -o $@

receiver: receiver.c
	gcc -pthread -Wall $< -o $@

clean:
	rm -f sender receiver *.o *~ core

