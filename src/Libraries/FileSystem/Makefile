SOURCES=FileSystem.cpp
OBJS=$(SOURCES:.cpp=.o)
CFLAGS= -DBOOST_LOG_DYN_LINK
LIBNAME=libFileSystem.a

%.o : %.cpp
	g++ -c $(CFLAGS) $(DEBUG) $(OPTIMIZE) $< -o $@

$(LIBNAME) : $(OBJS)
	ar rcs $@ $^

clean : 
	rm -rf $(OBJS) $(LIBNAME)

