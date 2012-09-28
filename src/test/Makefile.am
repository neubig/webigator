WEBIGATH = test-base.h test-util.h test-server.h test-data-store.h test-text-classifier.h

AM_CPPFLAGS = -I$(srcdir)/../include -DPKGDATADIR='"$(pkgdatadir)"'

bin_PROGRAMS = test-webigator

test_webigator_SOURCES = test-webigator.cc ${WEBIGATH}
test_webigator_LDADD = ../lib/libwebigator.la -lboost_regex -lxmlrpc++ -lxmlrpc_server_abyss++ -lxmlrpc_server++ -lxmlrpc_client++
