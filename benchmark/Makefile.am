TESTS = roq-kraken-futures-benchmark
bin_PROGRAMS = roq-kraken-futures-benchmark
roq_kraken_futures_benchmark_LDADD = \
	$(top_builddir)/src/roq/kraken_futures/json/libroq-kraken-futures-json.la \
	$(DATE_LIBS) \
	$(ROQ_API_LIBS) \
	$(ROQ_CORE_LIBS) \
	$(FMT_LIBS) \
	$(BENCHMARK_LIBS)  \
	$(RT_LIBS) \
	-lpthread
roq_kraken_futures_benchmark_CFLAGS = -pthread
roq_kraken_futures_benchmark_CPPFLAGS = \
	-I$(top_builddir)/src \
	$(ROQ_API_CFLAGS) \
	$(ROQ_CORE_CFLAGS) \
	$(BENCHMARK_CFLAGS) \
	$(FMT_CFLAGS) \
	$(DATE_CFLAGS)
nodist_roq_kraken_futures_benchmark_SOURCES = \
	main.cpp
