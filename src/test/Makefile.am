NAFILTH = test-webigator.h test-base.h

AM_CPPFLAGS = -I$(srcdir)/../include -DPKGDATADIR='"$(pkgdatadir)"'

bin_PROGRAMS = test-webigator

test_webigator_SOURCES = test-webigator.cc ${KYTH}
test_webigator_LDADD = ../lib/libwebigator.la -lboost_regex -lxmlrpc++ -lxmlrpc_server_abyss++ -lxmlrpc_server++ -lxmlrpc_client++

