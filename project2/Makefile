all:	peer tracker

peer: peer.c
	gcc -pthread -Wall $< -o build/$@

tracker: tracker.c
	gcc -pthread -Wall $< -o build/$@

clean:
	rm -f build/peer build/tracker *.o *~ core

