   CFLAGS = -g `pkg-config --cflags freetype2`
GL_CFLAGS = -g
#GL_CFLAGS = -g `pkg-config --cflags freetype2`
  LDFLAGS =
     LIBS = -lm
#     LIBS = -lm `pkg-config --libs freetype2`
  GL_LIBS = -lGL -lGLU -lGLEW -lglut

%.o:%.cpp
	g++ -c $(GL_CFLAGS) $<

stewart: stewart.o popen2.o
	g++ -o $@ $(LDFLAGS) $^ $(LIBS) $(GL_LIBS)

clean:
	rm -f *.o

veryclean: clean
	rm -f stewart
