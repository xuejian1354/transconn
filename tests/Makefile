TARGET  := 
SRC_FILES := 

INCLUDE +=
LDFLAGS +=

all:$(TARGET)
.PHONY: all clean

$(TARGET):$(SRC_FILES)
	$(TARGET_CC) -o $@ -c $^ $(DMACRO) $(INCLUDE) $(LDFLAGS)

clean:
	(find -name "*.o" | xargs $(RM)) && $(RM) $(TARGET)
