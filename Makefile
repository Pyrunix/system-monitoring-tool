CC = gcc
CCFLAGS = -Wall

.PHONY: all
all: a3 stats

a3: a3.o
	$(CC) $(CCFLAGS) $< -o $@

a3.o: a3.c 
	$(CC) $(CCFLAGS) -c $<

stats: stats_function.o
	$(CC) $(CCFLAGS) $< -o $@

stats_function.o: stats_function.c
	$(CC) $(CCFLAGS) -c $<

.PHONY: clean
clean:
	rm *.o stats