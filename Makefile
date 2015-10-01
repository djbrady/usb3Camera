# threadedCapture makefile
# To compile the debug verison need to overwrite CXXFLAGS variable to include -ggdb

CC = g++ -std=c++11 -pthread
OUTPUTNAME = threadedCapture${D}
INCLUDE = -I../../include -I/usr/include/flycapture
LIBS = -L/usr/src/flycapture/lib -lflycapture${D} 

OUTDIR = ~/bin

OBJS = threadedCapture.o

${OUTPUTNAME}: ${OBJS}
	${CC} -o ${OUTPUTNAME} ${OBJS} ${LIBS} ${COMMON_LIBS} 
	mv ${OUTPUTNAME} ${OUTDIR}

%.o: %.cpp
	${CC} ${CFLAGS} ${INCLUDE} -c $*.cpp
	
clean_obj:
	rm -f ${OBJS}	@echo "all cleaned up!"

clean:
	rm -f ${OUTDIR}/${OUTPUTNAME} ${OBJS}	@echo "all cleaned up!"
