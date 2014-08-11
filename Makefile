TARGET := 	ServerControlDaemon.cpp
ADDFILES :=	wol.c pinglib.cpp filehandling.c
SOURCE :=	$(TARGET) $(ADDFILES)
DEST :=		$(TARGET).exe


CC :=		g++

FLAG_WARN :=	-W -Wall -Wswitch -Wformat -Wchar-subscripts \
		-Wparentheses -Wmultichar -Wtrigraphs -Wpointer-arith \
		-Wcast-align -Wreturn-type -Wno-unused-function -Wno-unused-parameter \
		-Wno-reorder
FLAG_OPT := 	-fno-strict-aliasing -O3
FLAG_C :=	-fPIC -DUNIX


FLAGS_C :=	$(FLAG_C) $(FLAG_OPT) $(FLAG_WARN)

PATH_INCL := 	-I/usr/include -I/usr/local/include 
PATH_LIBS :=	-L/usr/lib/ -L/usr/local/lib/
PATHS :=	$(PATH_INCL) $(PATH_LIBS)

LIB_TMP :=	-loping
LIBRARIES :=	$(LIB_TMP)


default: main


run: $(DEST)
	sudo ./$(DEST)
	
install: $(DEST)
	sudo cp $(DEST) /usr/sbin/servercontroldaemon
	sudo chmod +x /usr/sbin/servercontroldaemon
	
start:
	sudo /etc/init.d/servercontroldaemon start
	
restart: 
	sudo killall servercontroldaemon
	sudo /etc/init.d/servercontroldaemon start
	
stop:
	sudo killall servercontroldaemon

main: clean
	$(CC) $(FLAGS_C) -o $(DEST) $(SOURCE) $(PATHS) $(LIBRARIES)


clean:
	rm -f *.o $(DEST) *log
.PHONY: clean
