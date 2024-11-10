# makefile
TARGET = rtsp_stream_server
SRC = rtsp_stream_server.c

CFLAGS = $(shell pkg-config --cflags gstreamer-1.0 gstreamer-rtsp-server-1.0 glib-2.0)
LIBS = $(shell pkg-config --libs gstreamer-1.0 gstreamer-rtsp-server-1.0 glib-2.0)

$(TARGET): $(SRC)
	$(CC) -o $(TARGET) $(SRC) $(CFLAGS) $(LIBS)

clean:
	rm -f $(TARGET)

