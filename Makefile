CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2 -Iinclude
LDFLAGS = 

# Directories
SRCDIR = src
INCDIR = include
OBJDIR = obj

# Source files
SOURCES = $(SRCDIR)/main.c $(SRCDIR)/image_encode.c $(SRCDIR)/image_decode.c $(SRCDIR)/stego_bits.c $(SRCDIR)/video.c
HEADERS = $(INCDIR)/common.h $(INCDIR)/encode.h $(INCDIR)/decode.h $(INCDIR)/types.h $(INCDIR)/video.h
OBJECTS = $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

# Target executable
TARGET = stego

# Default target
all: $(OBJDIR) $(TARGET)

# Create directories
$(OBJDIR):
	mkdir -p $(OBJDIR)

# Link object files to create executable
$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "Build complete: $@"

# Compile source files to object files
$(OBJDIR)/%.o: $(SRCDIR)/%.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean build artifacts
clean:
	rm -rf $(OBJDIR) $(BINDIR)
	@echo "Clean complete"

# Remove everything and rebuild
rebuild: clean all

# Phony targets (not actual files)
.PHONY: all clean rebuild

# Help target
help:
	@echo "Available targets:"
	@echo "  all     - Build the executable (default)"
	@echo "  clean   - Remove object files and executable"
	@echo "  rebuild - Clean and rebuild"
	@echo "  help    - Show this help message"
	@echo ""
	@echo "Output: stego"
	@echo "Objects: obj/"
