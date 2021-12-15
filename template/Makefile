TAG=./bin/test
SRC= $(wildcard source/*.c)
OBJ=$(SRC:%.c=%.o)
CC=arm-linux-gcc

override CONFIG += -I./include  -L./library -ljpeg -lfont -lm -lqrencode

$(TAG):$(OBJ)
	$(CC)  $(^) -o $(@) $(CONFIG)
	scp $(TAG) root@192.168.14.53:~/code/

%.o:%.c
	$(CC) $< -o $(@) $(CONFIG) -c 

clean:
	$(RM) ./bin/*  ./source/*.o

.PHONY:clean