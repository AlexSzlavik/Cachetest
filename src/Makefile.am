bin_PROGRAMS = cachetest launcher
launcher_SOURCES = launcher.cc
if PAPI
LDADD = @PAPI_DIR@/lib/libpapi.a
cachetest_SOURCES = cachetest.cc Distribution.cc Buffer.cc genzipf.cc Perf_PAPI.cc Perf.cc Small_buffer.cc Large_buffer.cc
else
cachetest_SOURCES = cachetest.cc Distribution.cc Buffer.cc genzipf.cc Perf_Cachetest.cc Perf.cc Small_buffer.cc Large_buffer.cc
endif


AM_CPPFLAGS = -I./include
