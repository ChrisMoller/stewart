   CFLAGS = -O2 `pkg-config --cflags freetype2`
GL_CFLAGS = -)2
  LDFLAGS =
     LIBS = -lm
#     LIBS = -lm `pkg-config --libs freetype2`
  GL_LIBS = -lGL -lGLU -lGLEW -lglut
  SOURCES = LICENSE  Makefile  popen2.cpp  popen2.h  README.md  stewart.cpp

%.o:%.cpp
	g++ -c $(GL_CFLAGS) $<

stewart: stewart.o popen2.o
	g++ -o $@ $(LDFLAGS) $^ $(LIBS) $(GL_LIBS)

clean:
	rm -f *.o

veryclean: clean
	rm -f stewart

stewart.zip: $(SOURCES)
	zip -u $@ $^

zip: stewart.zip
