PROG:=affine
AUTO_SCHEDULE:=false
include ../../common.mk

${PROG}_test2: ${PROG}_test2.cc ${PROG}.h ${PROG}.a
	g++ -D_BPG_API -I . ${CXXFLAGS} $< -o $@ ${PROG}.a -ldl -lpthread -ljpeg -lpng -lbpg_enc -lbpg_dec -lx265_8bit -lx265_10bit -lx265_12bit

clean2:
	rm -rf ${PROG}_test2 *.png
