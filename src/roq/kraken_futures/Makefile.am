SUBDIRS = json

bin_PROGRAMS = roq-kraken-futures
roq_kraken_futures_LDFLAGS = \
	$(PTHREAD_CFLAGS) \
	$(top_builddir)/src/roq/kraken_futures/json/libroq-kraken-futures-json.la \
	$(ROQ_SERVER_LIBS) \
	$(ROQ_CORE_LIBS) \
	$(ROQ_API_LIBS) \
	$(ROQ_LOGGING_LIBS) \
	$(BROTLI_LIBS) \
	$(HTTP_PARSER_LIBS) \
	$(FMT_LIBS) \
	$(ABSEIL_LIBS) \
	$(LIBEVENT_LIBS) \
	-levent_openssl \
	$(OPENSSL_LIBS) \
	$(CROSSGUID_LIBS) \
	$(CRYPTO_LIBS) \
	$(PTHREAD_LIBS) \
	$(STDCXX_FS_LIBS) \
	$(RT_LIBS) \
	$(GFLAGS_LIBS)
roq_kraken_futures_CPPFLAGS = \
	-I$(top_srcdir)/src \
	$(PTHREAD_CFLAGS) \
	$(ABSEIL_CFLAGS) \
	$(GFLAGS_CFLAGS) \
	$(OPENSSL_CFLAGS) \
	$(LIBEVENT_CFLAGS) \
	$(FMT_CFLAGS) \
	$(HTTP_PARSER_CFLAGS) \
	$(BROTLI_CFLAGS) \
	$(ROQ_LOGGING_CFLAGS) \
	$(ROQ_API_CFLAGS) \
	$(ROQ_CORE_CFLAGS) \
	$(ROQ_SERVER_CFLAGS)
AUTOGEN_CPP = \
	web_socket_state.cpp
AUTOGEN_H = \
	web_socket_state.h
CLEANFILES = $(AUTOGEN_CPP) $(AUTOGEN_H)
BUILT_SOURCES = $(CLEANFILES) .gitignore
roq_kraken_futures_SOURCES = \
	$(AUTOGEN_CPP) \
	application.cpp \
	config.cpp \
	gateway.cpp \
	options.cpp \
	random.cpp \
	rest.cpp \
	web_socket.cpp \
	main.cpp

.gitignore: Makefile.am
	echo $(AUTOGEN_CPP) $(AUTOGEN_H) | tr " " "\n" | sort > $@

.json.h:
	@abs_top_builddir@/autogen/autogen.py \
		--namespace=roq/kraken_futures \
		--output-type state \
		--file-type h \
		$< > $@

.json.cpp:
	@abs_top_builddir@/autogen/autogen.py \
		--namespace=roq/kraken_futures \
		--output-type state \
		--file-type cpp \
		$< > $@
