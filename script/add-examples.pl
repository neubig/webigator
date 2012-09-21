#!/usr/bin/perl

use strict;
use utf8;
use Encode;
use XMLRPC::Lite;
use Getopt::Long;
use List::Util qw(sum min max shuffle);
binmode STDIN, ":utf8";
binmode STDOUT, ":utf8";
binmode STDERR, ":utf8";

if(@ARGV != 1) {
    print STDERR "Usage: $0 TEXT_FILE\n";
    exit 1;
}

my $SERVER = "localhost:9597";
my $result = GetOptions (
    "server=s" => \$SERVER,
); # flag

my $url = "http://$SERVER/RPC2";
my $proxy = XMLRPC::Lite->proxy($url);

open FILE, "<:utf8", $ARGV[0] or die "Couldn't open $ARGV[0]\n";

while(<FILE>) {
    chomp;
    my @arr = split(/\t/);
    die "Bad input line (tweet data must be 4 columns\n$_" if(@arr != 4);
    my ($tid, $uid, $date, $text) = @arr;
    
    # Work-around for XMLRPC::Lite bug
    # $encoded = SOAP::Data->type(string => Encode::encode("utf8",$text));
    my $encoded = SOAP::Data->type(string => $text);
    
    my %param = ("text" => $encoded );
    my %param = ("id" => $tid );
    $result = $proxy->call("addunlab",\%param)->result;
    print "$result\n";
    # $result = $proxy->call("add_unlab_examp",\%param)->result;
    # print $result->{'text'} . "\n";

}
