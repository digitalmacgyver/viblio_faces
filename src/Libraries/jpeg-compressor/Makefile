SOURCES=jpge.cpp
OBJS=$(SOURCES:.cpp=.o)
CFLAGS= -DBOOST_LOG_DYN_LINK
LIBNAME=libJpegcompressor.a

%.o : %.cpp
	g++ -c $(CFLAGS) $< -o $@

$(LIBNAME) : $(OBJS)
	ar rcs $@ $^

clean : 
	rm -rf $(OBJS) $(LIBNAME)

