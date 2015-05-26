#CC=gcc -g3 -Wall -rdynamic
#AFLAGS=-c -DDEBUG -lmysqlclient-lpthread -lccl -lz 
#LDFLAGS=-DDEBUG -lmysqlclient -lpthread -lccl -lz

#CC=arm-none-linux-gnueabi-gcc -g3 -rdynamic -Wall 
CC=arm-arago-linux-gnueabi-gcc -g3 -rdynamic -Wall 
AFLAGS=-c -DDEBUG -lmysqlclient_r -lpthread -lccl -lz 
LDFLAGS=-DDEBUG -lmysqlclient_r -lpthread -lccl -lz

SRC=${wildcard *.c}
OBJS=${patsubst *.c,*.o,$(SRC)}
MOBJS=MoniterKey
$(MOBJS):$(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)
$(OBJS):%.o:%.c
	$(CC) $(AFLAGS) $< -o $@
.PHONY:clean
clean:
	rm $(MOBJS)



#CC=arm-none-linux-gnueabi-gcc -g3 -rdynamic -Wall 
