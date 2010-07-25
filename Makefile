CPPFLAGS = -Wall
OBJS = core/Binding.o core/Socket.o stomp/StompSocket.o stomp/StompController.o stomp/StompClient.o stomp/StompProtocol.o stomper.o

stomper: $(OBJS)
	$(CC) -o stomper $(OBJS) -lstdc++

clean:
	rm -f *.o
	rm -f core/*.o
	rm -f stomp/*.o
	rm -f stomper
