#!/usr/bin/perl

use strict;
use utf8;
use Encode;
use XMLRPC::Lite;
use Data::Dumper;
use Getopt::Long;
use List::Util qw(sum min max shuffle);
binmode STDIN, ":utf8";
binmode STDOUT, ":utf8";
binmode STDERR, ":utf8";

if(@ARGV != 0) {
    print STDERR "Usage: $0\n";
    exit 1;
}

my $SERVER = "127.0.0.1:9597";
my $result = GetOptions (
    "server=s" => \$SERVER,
); # flag

my $url = "http://$SERVER/RPC2";
print "$url\n";
my $proxy = XMLRPC::Lite->proxy($url);
print "$proxy\n";
    
$result = $proxy->call("pop_best")->result;
die "Popping example failed: $!" if(!defined($result));
print Dumper($result)."\n";
# $result = $proxy->call("add_unlab_examp",\%param)->result;
# print $result->{'text'} . "\n";
