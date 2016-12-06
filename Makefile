INCLUDES=-I/opt/local/include
LIBS=-L/opt/local/lib

# Comment out on Win32
DEFINES=-DHAVE_MADVISE=1

all: mthasher

mthasher: mthasher.cpp
	$(CC) $(CFLAGS) $(INCLUDES) $(LIBS) $(DEFINES) -o mthasher mthasher.cpp -lboost_program_options-mt -lboost_thread-mt -lboost_iostreams-mt -lboost_system-mt -lstdc++

check: mthasher
	./mthasher -i random_junk -o random_junk.crc32
	diff -u random_junk.crc32.ref random_junk.crc32
	
clean:
	rm -f mthasher
	rm -f random_junk.crc32

