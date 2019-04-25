TARGET = oss user
flag = -pthread

all: $(TARGET)

oss: oss.o
	gcc -o$@ $^ $(flag)

user: user.o
	gcc -o$@ $^ $(flag)

%.o: %.c
	gcc -c -o$@ $<

clean:
	rm -f *.o $(TARGET)

