AM_CPPFLAGS = -I$(srcdir)/../include

bin_PROGRAMS = webigator webigator-client

webigator_SOURCES = webigator.cc
webigator_LDADD = ../lib/libwebigator.la -lxmlrpc++ -lxmlrpc_server -lxmlrpc -lxmlrpc_server_abyss++ -lxmlrpc_abyss -lxmlrpc_server_abyss -lxmlrpc_client++ -lxmlrpc_server_cgi -lxmlrpc_client -lxmlrpc_util -lxmlrpc_cpp -lxmlrpc_xmlparse -lxmlrpc_server++ -lxmlrpc_xmltok

webigator_client_SOURCES = webigator-client.cc
webigator_client_LDADD = ../lib/libwebigator.la -lxmlrpc++ -lxmlrpc_server_abyss++ -lxmlrpc_client++
