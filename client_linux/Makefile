DIR_TARGET := bin
DIR_SRC := src
DIR_INIT := init.d
FILE_TARGET := shutdowndaemon
FILE_SRC := shutdowndaemon.cpp
FILE_INIT := shutdowndaemon
INIT := $(DIR_INIT)/$(FILE_INIT)
TARGET := 	$(DIR_TARGET)/$(FILE_TARGET)
MAINFILE := $(DIR_SRC)/shutdowndaemon.cpp
ADDFILES :=
SOURCE :=	$(MAINFILE) $(ADDFILES)


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


default:
	$(CC) $(FLAGS_C) -o $(TARGET) $(SOURCE) $(PATHS) $(LIBRARIES)
	
install: stop $(TARGET)
	sudo cp $(INIT) /etc/init.d/$(FILE_INIT)
	sudo chmod +x /etc/init.d/$(FILE_INIT)
	sudo cp $(TARGET) /usr/sbin/$(FILE_TARGET)
	sudo chmod +x /usr/sbin/$(FILE_TARGET)
	
remove: stop
	sudo rm /etc/init.d/$(FILE_INIT)
	sudo rm /usr/sbin/$(FILE_TARGET)
	
start: /etc/init.d/$(FILE_INIT)
	sudo service $(FILE_TARGET) start
	
restart: /etc/init.d/$(FILE_INIT)
	sudo service $(FILE_TARGET) restart
	
stop:
	-sudo killall -q $(FILE_TARGET)

$(TARGET):
	$(CC) $(FLAGS_C) -o $(TARGET) $(SOURCE) $(PATHS) $(LIBRARIES)


clean:
	rm -f *.o $(TARGET) *log
.PHONY: clean
