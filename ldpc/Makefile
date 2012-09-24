LDPC_LIBRARY = ldpclib/libldpc.o
all: libldpc sender receiver

libldpc:
	cd ldpclib/src; make; cd ../..
	
sender: sender.o socketwrapper.o serializer.o ldpcuser.o
	g++ -g -Wall -o sender sender.o socketwrapper.o serializer.o ldpcuser.o ${LDPC_LIBRARY} -lpthread

receiver: receiver.o socketwrapper.o serializer.o ldpcuser.o
	g++ -g -Wall -o receiver receiver.o socketwrapper.o serializer.o ldpcuser.o ${LDPC_LIBRARY} -lpthread
	
sender.o: sender.cpp sender.h packetformat.h settings.h exceptions.h socketwrapper.h serializer.h
	g++ -g -c -Wall -o sender.o sender.cpp
	
socketwrapper.o: socketwrapper.cpp socketwrapper.h exceptions.h
	g++ -g -c -Wall -o socketwrapper.o socketwrapper.cpp
	
serializer.o: serializer.cpp serializer.h packetformat.h
	g++ -g -c -Wall -o serializer.o serializer.cpp

receiver.o: receiver.cpp serializer.h packetformat.h receiver.h settings.h socketwrapper.h exceptions.h
	g++ -g -c -Wall -o receiver.o receiver.cpp

ldpcuser.o: ldpcuser.cpp ldpcuser.h
	g++ -g -c -Wall -o ldpcuser.o ldpcuser.cpp

clean:
	rm -rf *.o
	rm sender
	rm receiver
	cd ldpclib/src; make clean; cd ../..
	