#!/usr/bin/perl

use strict;
use utf8;
use Encode;
use XML::RPC;
use SOAP::Lite;
use Getopt::Long;
use Data::Dumper;
use List::Util qw(sum min max shuffle);
binmode STDOUT, ":utf8";
binmode STDERR, ":utf8";

if(@ARGV != 1) {
    print STDERR "Usage: $0 TEXT_FILE\n";
    exit 1;
}

my $SERVER = "localhost:9597";
my $TOKENIZE = "char";

my $result = GetOptions (
    "server=s" => \$SERVER, # Which server to use
    "tokenize=s" => \$TOKENIZE, # What type of tokenization to do? Character or word
); # flag

my $url = "http://$SERVER/RPC2";
my $xmlrpc = XML::RPC->new($url);

open FILE, "<:utf8", $ARGV[0] or die "Couldn't open $ARGV[0]\n";

while(<FILE>) {
    chomp;
    my @arr = split(/\t/);
    die "Bad input line (tweet data must be 4 columns\n$_" if(@arr != 4);
    my ($tid, $uid, $date, $text) = @arr;
    
    # Convert the text into a character string
    if($TOKENIZE eq "char") {
        $text =~ s/(.)/$1 /g;
        $text =~ s/ $//g;
    }

    utf8::encode($text); 
    $result = $xmlrpc->call("add_unlabeled", {id => int($tid), text => $text});

    die "Adding example failed: $!" if(!defined($result));
    print Data::Dumper->Dump([ $result ]);
}
