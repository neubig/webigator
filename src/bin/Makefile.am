AM_CPPFLAGS = -I$(srcdir)/../include

bin_PROGRAMS = webigator webigator-client

webigator_SOURCES = webigator.cc
webigator_LDADD = ../lib/libwebigator.la -lxmlrpc++ -lxmlrpc_server_abyss++ -lxmlrpc_server++ -lxmlrpc_client++

webigator_client_SOURCES = webigator-client.cc
webigator_client_LDADD = ../lib/libwebigator.la -lxmlrpc++ -lxmlrpc_server_abyss++ -lxmlrpc_client++
