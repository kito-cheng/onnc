ONNC_INCLUDES = -I${abs_top_srcdir}/tools/onnc \
	@LIBONNC_INCLUDES@ @SKYPAT_INCLUDES@

ANDROID_CPPFLAGS=-Waddress -Wchar-subscripts -Wcomment -Wformat -Wparentheses -Wreorder -Wreturn-type -Wsequence-point -Wstrict-aliasing -Wstrict-overflow=1 -Wswitch -Wtrigraphs -Wuninitialized -Wunknown-pragmas -Wunused-function -Wunused-label -Wunused-value -Wunused-variable -Wvolatile-register-var -Wno-return-stack-address

ONNC_CPPFLAGS = -O0 -g3 \
	-DUNITTEST=1 \
	-DTOPDIR=\"${abs_top_srcdir}\" \
	-DBUILDDIR=\"${abs_top_builddir}\"

if ENABLE_WERROR
ONNC_CPPFLAGS += -Werror
endif

AM_CPPFLAGS = ${ONNC_INCLUDES} ${ONNC_CPPFLAGS} ${ANDROID_CPPFLAGS}

bin_PROGRAMS = onnc

onnc_LDFLAGS = @LIBONNC_LDFLAGS@

onnc_LDADD = @LIBONNC_LIBS@ @SKYPAT_LIBS@ -lglog -lprotobuf -lboost_system

nodist_onnc_SOURCES = main.cpp \
	ONNCJITApp.cpp \
	ONNCJITConfig.cpp

if HAVE_PTHREADS
onnc_LDADD += -lpthread
endif
