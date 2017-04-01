DIR_TARGET := bin
DIR_SRC := src
DIR_INIT := systemd
FILE_TARGET := servercontroldaemon
FILE_INIT := servercontroldaemon.service
INIT := $(DIR_INIT)/$(FILE_INIT)
TARGET := 	$(DIR_TARGET)/$(FILE_TARGET)
MAINFILE := $(DIR_SRC)/main.cpp
ADDFILES :=	$(DIR_SRC)/wol.c $(DIR_SRC)/pinglib.cpp $(DIR_SRC)/filehandling.c $(DIR_SRC)/logger.cpp $(DIR_SRC)/server_control.cpp
SOURCE :=	$(MAINFILE) $(ADDFILES)


CC :=		g++ -std=c++0x

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
	sudo cp $(INIT) /etc/systemd/system/$(FILE_INIT)
	sudo chmod +x /etc/systemd/system/$(FILE_INIT)
	sudo cp $(TARGET) /usr/sbin/$(FILE_TARGET)
	sudo chmod +x /usr/sbin/$(FILE_TARGET)
	
remove: stop
	sudo rm /etc/systemd/system/$(FILE_INIT)
	sudo rm /usr/sbin/$(FILE_TARGET)
	
start: /etc/systemd/system/$(FILE_INIT)
	sudo systemctl start $(FILE_INIT)
	
restart: /etc/systemd/system/$(FILE_INIT)
	sudo systemctl restart $(FILE_INIT)
	
stop: /etc/systemd/system/$(FILE_INIT)
	sudo systemctl stop $(FILE_INIT)

status: /etc/systemd/system/$(FILE_INIT)
	sudo systemctl status $(FILE_INIT)

$(TARGET):
	$(CC) $(FLAGS_C) -o $(TARGET) $(SOURCE) $(PATHS) $(LIBRARIES)


clean:
	rm -f *.o $(TARGET) *log
.PHONY: clean
