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
my $KEYWORD_UPDATE = 5;
my $MIN_LEN = 10;
my $REST = 0;
my $result = GetOptions (
    "server=s" => \$SERVER,         # Which server to use
    "tokenize=s" => \$TOKENIZE,     # What type of tokenization to do? Character or word
    "data=s" => \$DATA,             # The location of the data
    "keyword-update=i" => \$KEYWORD_UPDATE, # How frequently to update the keywords
    "min-len=i" => \$MIN_LEN,       # The minimum length of a tweet
    "rest=i" => \$REST,             # Rest for this many seconds every keyword update
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
    eval { $result = $xmlrpc->call("get_keywords", {task_id => "-1"}); };
    $xmlrpc->{tpp}->set( utf8_flag => 0 ); # This is a hack so UTF works
    return @{["サーバ${SERVER}への接続が失敗しました"]} if ($@);
    return @{[$result->{"faultString"}]} if (UNIVERSAL::isa($result,'HASH') and $result->{"faultString"});
    my @keywords = map { detokenize($_) } @$result;
    return @keywords ? "(".join("|", @keywords).")" : "";
}
my $regex = "";

##### If there is a data file to use, add the examples to the server
my $last_update = 0;
while(1) {
    print "Loading data from $DATA (. == 5,000 sentences, ! == 50,000 sentences)\n";
    my $lines = 0;
    my $checked = 0;
    open FILE, "<:utf8", $DATA or die "Couldn't open $DATA";
    while(<FILE>) {
        # Check to make sure we are OK
        chomp;
        my @arr = split(/\t/);
        die "Bad input line (tweet data must be 4 columns\n$_" if(@arr != 4);
        my ($tid, $uid, $date, $text) = @arr;
        # Remove control characters
        $text =~ tr/\000-\037/ /;
        # Update the keyword if necessary
        if(time() - $last_update > $KEYWORD_UPDATE) {
            $last_update = time();
            my $old_regex = $regex;
            $regex = get_keyword_regex();
            if($regex ne $old_regex) { print "New keywords: $regex\n"; }
            sleep $REST if ($REST and $lines);
        }
        # Skip this text if it is not necessary
        next if (length($text) < $MIN_LEN) or ($regex and not $text =~ m/$regex/);
        if(++$lines % 50000 == 0) { print "!"; }
        elsif($lines % 5000 == 0) { print "."; }
        # Convert the text into a character string
        $text = tokenize($text) if($TOKENIZE eq "char"); 
        utf8::encode($text); 
        $result = $xmlrpc->call("add_unlabeled", {id => int($tid), text => $text}); 
        die "Adding example failed: $! -- ".Dumper($result) if not defined($result) or (UNIVERSAL::isa($result,'HASH') and $result->{"faultString"});
    }
    print "\n";
}
