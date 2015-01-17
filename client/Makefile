INCLUDEFLAGS=-I/opt/vc/include -I/opt/vc/include/interface/vmcs_host/linux -I/opt/vc/include/interface/vcos/pthreads -I..
LIBFLAGS=-L/opt/vc/lib -lGLESv2 -lEGL -lbcm_host -lpthread  -ljpeg

digital_signage:
	gcc -Wall $(INCLUDEFLAGS) $(LIBFLAGS) -o  digital-signage digital-signage.c ../libshapes.o ../oglinit.o	

libshapes.o:
	cd .. ; make libshapes.o

oglinit.o:
	cd .. ; make oglinit.o