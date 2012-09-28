#!/usr/bin/perl

use strict;
use utf8;
use Encode;
use XML::RPC;
use SOAP::Lite;
use Getopt::Long;
use Data::Dumper;
use List::Util qw(sum min max shuffle);
binmode STDIN, ":utf8";
binmode STDOUT, ":utf8";
binmode STDERR, ":utf8";

$| = 1;

##### Get the options
my $SERVER = "localhost:9597";
my $TOKENIZE = "char";
my $DATA = "";
my $KEYWORD_UPDATE = 10000;
my $result = GetOptions (
    "server=s" => \$SERVER,         # Which server to use
    "tokenize=s" => \$TOKENIZE,     # What type of tokenization to do? Character or word
    "data=s" => \$DATA,             # The location of the data
    "keyword-update=i" => \$KEYWORD_UPDATE, # How frequently to update the keywords
);
if(@ARGV != 0) { print STDERR "Usage: $0\n"; exit 1; }
$DATA or die "Must define a data source using -data";

##### Open the connection to the server
my $url = "http://$SERVER/RPC2";
my $xmlrpc = XML::RPC->new($url);

##### Two functions to split sentences into characters and return them back
sub tokenize {
    $_ = shift;
    return join(" ", map { ($_ eq " ") ? "__" : $_ } split(//));
}
sub detokenize {
    $_ = shift;
    return join('', map { ($_ eq "__") ? " " : $_ } split(/ /));
}

##### Get a regular expression to indicate matching the keywords
sub get_keyword_regex {
    my $result;
    $xmlrpc->{tpp}->set( utf8_flag => 1 ); # This is a hack so UTF works
    eval { $result = $xmlrpc->call("get_keywords"); };
    $xmlrpc->{tpp}->set( utf8_flag => 0 ); # This is a hack so UTF works
    return @{["サーバ${SERVER}への接続が失敗しました"]} if ($@);
    return @{[$result->{"faultString"}]} if (UNIVERSAL::isa($result,'HASH') and $result->{"faultString"});
    my @keywords = map { detokenize($_) } @$result;
    return @keywords ? "(".join("|", @keywords).")" : "";
}
my $regex = "";

##### If there is a data file to use, add the examples to the server
while(1) {
    print "Loading data from $DATA (. == 10,000 sentences, ! == 100,000 sentences)\n";
    my $lines = 0;
    open FILE, "<:utf8", $DATA or die "Couldn't open $DATA";
    while(<FILE>) {
        if($lines % $KEYWORD_UPDATE == 0) {
            my $old_regex = $regex;
            $regex = get_keyword_regex();
            if($regex ne $old_regex) { print "New keywords: $regex\n"; }
        }
        if(++$lines % 100000 == 0) { print "!"; }
        elsif($lines % 10000 == 0) { print "."; }
        next if $regex and not m/$regex/;
        chomp;
        my @arr = split(/\t/);
        die "Bad input line (tweet data must be 4 columns\n$_" if(@arr != 4);
        my ($tid, $uid, $date, $text) = @arr;
        
        # Convert the text into a character string
        $text = tokenize($text) if($TOKENIZE eq "char"); 
        utf8::encode($text); 
        $result = $xmlrpc->call("add_unlabeled", {id => int($tid), text => $text}); 
        die "Adding example failed: $!" if not defined($result);
    }
    print "\n";
}
