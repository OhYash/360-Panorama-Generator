# build executables for the Stitcher and Cropper
CC = g++

# Requires OpenCV Library
PACKAGES = opencv
PKGCONFIG = pkg-config

LIBS = $(shell $(PKGCONFIG) --libs $(PACKAGES))
CPPFLAGS = $(shell $(PKGCONFIG) --cflags $(PACKAGES))

# The build Target Stitcher and cropper
STITCHER = stitcher
CROPPERE = cropp  # Because Cropper's executable has a name
CROPPER = cropper

all: $(STITCHER) $(CROPPER)

$(STITCHER): src/$(STITCHER).cpp
	$(CC) -o $(STITCHER) src/$(STITCHER).cpp $(CPPFLAGS) $(LIBS)
$(CROPPER): src/$(CROPPER).cpp
	$(CC) -o $(CROPPERE) src/$(CROPPER).cpp $(CPPFLAGS) $(LIBS)

clean:
	$(RM) $(STITCHER) $(CROPPERE)
