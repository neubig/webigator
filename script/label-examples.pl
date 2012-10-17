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
my $DATA_FILE = "";
my $SAVE_FILE = "";
my $RESCORE = 1;
my $MATCH_KEYWORDS = 1;
my $result = GetOptions (
    "server=s" => \$SERVER, # Which server to use
    "tokenize=s" => \$TOKENIZE, # What type of tokenization to do? Character or word
    "data_file=s" => \$DATA_FILE, # A file containing tweet data
    "save_file=s" => \$SAVE_FILE, # A file to save the labeled tweets
);
if(@ARGV != 0) { print STDERR "Usage: $0\n"; exit 1; }

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

##### First, add the task
my $task = $xmlrpc->call("add_task");
die "Adding task failed: ".$task->{"faultString"} if (UNIVERSAL::isa($task,'HASH') and $task->{"faultString"});
print "Obtained task $task\n";

##### Ask the user to enter keywords
my @keywords;
while(1) {
    # Ask the user to enter a keyword
    print "Enter a keyword (or just press 'enter' to finish): \n";
    $_ = <STDIN>;
    chomp;
    last if not $_;
    push @keywords, $_;
    # Convert the text into a character string
    $_ = tokenize($_) if($TOKENIZE eq "char");
    # Dispatch a request to add a keyword
    utf8::encode($_); 
    $result = $xmlrpc->call("add_keyword", {id => -1, text => $_, lab => 1, task => $task});
    die "Adding keyword failed: ".$result->{"faultString"} if ($result != 1);
}
my $regex = "(".join("|", @keywords).")";

##### If there is a data file to use, add the examples to the server
sub add_examples {
    print "Loading data from $DATA_FILE (. == 10,000 sentences, ! == 100,000 sentences)\n";
    my $lines = 0;
    open FILE, "<:utf8", $DATA_FILE or die "Couldn't open $DATA_FILE\n";
    while(<FILE>) {
        if(++$lines % 100000 == 0) { print "!"; }
        elsif($lines % 10000 == 0) { print "."; }
        next if $MATCH_KEYWORDS and not m/$regex/;
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
add_examples() if($DATA_FILE);

##### Open the save file
if($SAVE_FILE) {
    open SAVE, ">:utf8", $SAVE_FILE or die "Couldn't open $SAVE_FILE\n";
}

##### Now, start labeling useful examples
while(1) {

    # Get an example
    $xmlrpc->{tpp}->set( utf8_flag => 1 ); # This is a hack so UTF works
    $result = $xmlrpc->call("pop_best");
    # print Data::Dumper->Dump([ $result ]);
    # Exit if there was none found
    if(not $result) {
        print "No more examples now! Try again later.\n";
        last;
    }
    
    # Get the data and ask for the user's response
    my $text = $result->{"text"};
    $text = detokenize($text) if($TOKENIZE eq "char");
    print "$text\n"; 
    my $label;
    while(1) { 
        print "Is this tweet useful? y/n: ";
        $label = <STDIN>; chomp $label;
        last if $label =~ /^[YNyn]$/;
    }
    
    # Record the user's response on the server
    $xmlrpc->{tpp}->set( utf8_flag => 0 ); # This is a hack so UTF works
    $text = tokenize($text) if($TOKENIZE eq "char"); 
    utf8::encode($text); 
    $result->{"lab"} = (($label =~ /^[Yy]/) ? 1 : 0);
    $result->{"text"} = $text;
    my $addresult = $xmlrpc->call("add_labeled", $result); 
    die "Adding labeled example failed: $!" if not defined($result);

    # Also save it in a file
    if($SAVE_FILE) {
        print SAVE "".$result->{"id"}."\t".$result->{"lab"}."\t".$result->{"text"}."\n";
    }

    # Update the labeled examples
    if($RESCORE) {
        $result = $xmlrpc->call("rescore"); 
        die "Rescoring the cache failed: $!" if not defined($result);
    }

}

close SAVE;
