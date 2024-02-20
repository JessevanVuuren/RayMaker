CC=emcc

SRC_DIR = ./src

LIBRARY_DIR = -L./lib/raylib -L./lib/stb
INCLUDE_DIR = -I./lib/raylib -I./lib/stb

HTML_FILE = --shell-file
RESOURCES = --preload-file ./build/resources@/resources

LIBS = ./lib/raylib/libraylib.a

CFLAGS = -g -gsource-map -Os -Wall

LFLAGS= -s USE_GLFW=3 -s ASYNCIFY -s TOTAL_MEMORY=1610612736 -s TOTAL_STACK=512mb -s FORCE_FILESYSTEM=1 -s 'EXPORTED_RUNTIME_METHODS=["cwrap", "FS"]' -s EXPORTED_FUNCTIONS='["_main"]'

OUTPUT_DIR_DEV=./debug
OUTPUT_DIR_PRO=./web

dev: LFLAGS += -s ASSERTIONS=1 -s SAFE_HEAP=1 --profiling
dev: HTML_FILE += ./build/static/index.html

pro: HTML_FILE += ./build/static/index.html

SOURCES=$(wildcard $(SRC_DIR)/*.c)

TARGET=index.html


dev:
	mkdir -p $(OUTPUT_DIR_DEV)
	mkdir -p $(OUTPUT_DIR_DEV)/src

	$(CC) $(CFLAGS) -o $(OUTPUT_DIR_DEV)/$(TARGET) $(SOURCES) $(INCLUDE_DIR) $(LIBRARY_DIR) $(LIBS) $(RESOURCES) $(HTML_FILE) $(LFLAGS)
	cp -r ./src/* ./debug/src
	cp -r ./build/static/js ./debug
	cp -r ./build/static/style.css ./debug
	


pro:
	mkdir -p $(OUTPUT_DIR_PRO)
	echo "" > $(OUTPUT_DIR_PRO)/$(TARGET)

	$(CC) -o $(OUTPUT_DIR_PRO)/$(TARGET) $(SOURCES) $(INCLUDE_DIR) $(LIBRARY_DIR) $(LIBS) $(RESOURCES) $(HTML_FILE) $(LFLAGS) 


clean-dev:
	rm -rf $(OUTPUT_DIR_DEV)

clean-pro:
	rm -rf $(OUTPUT_DIR_PRO)

	