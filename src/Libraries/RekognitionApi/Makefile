SOURCES=\
Base64Codec.cpp\
json/json_reader.cpp\
json/json_value.cpp\
json/json_writer.cpp\
RESTapi.cpp

OBJS=$(SOURCES:.cpp=.o)
CFLAGS=-std=c++0x -I../ -DBOOST_LOG_DYN_LINK
LIBNAME=libRekognitionApi.a

%.o : %.cpp
	g++ -c $(CFLAGS) $(DEBUG) $(OPTIMIZE) $< -o $@

$(LIBNAME) : $(OBJS)
	ar rcs $@ $^

clean :
	rm -rf $(LIBNAME) $(OBJS)

