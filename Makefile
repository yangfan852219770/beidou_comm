# Makefile
objs = test.o beidou_read.o minmea.o
test: $(objs)
	cc -o $@ $^ -lrt
test.o : beidou_read.h minmea.h
beidou_read.o: beidou_read.h minmea.h
minmea.o: minmea.h
.PHONY : clean
clean : 
	-rm *.o