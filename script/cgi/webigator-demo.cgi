#!/usr/bin/perl -w

use strict;
use utf8;
use Encode;
use CGI;
# use CGI::Carp qw(fatalsToBrowser);
use XML::RPC;
use Data::Dumper;
use HTML::Template;

binmode STDIN,  ":utf8";
binmode STDOUT, ":utf8";
binmode STDERR, ":utf8";

require "settings.pl";
require "functions.pl";

our $UILANG;
our $SERVER;
our $TOP_DIR;

##### Get the connection to the server
my $url = "http://$SERVER/RPC2";
my $xmlrpc = XML::RPC->new($url, utf8_flag => 1);

##### Get the parameter list
my $cgi = new CGI;
my $params = {map{$_, Encode::decode("utf8", $cgi->Vars->{$_})} $cgi->Vars};

my $error = "";

my @list;

##### Get the list of tasks
my $result;
eval { $result = $xmlrpc->call("get_tasks"); };
if($@) {
    if($UILANG eq "ja") {
        $error = "サーバ${SERVER}への接続が失敗しました！サーバを立ち上げる必要があります。";
    } else {
        $error = "Could not connect to server ${SERVER}! The server must be started.";
    }
} elsif(UNIVERSAL::isa($result,'HASH') and $result->{"faultString"}) {
    if($UILANG eq "ja") {
        $error = "サーバ${SERVER}からのタスク取得が失敗しました：".$result->{"faultString"};
    } else {
        $error = "Could not receive a task from ${SERVER}: ".$result->{"faultString"};
    }
} else {
    for(@$result) {
        push @list, {
            keywords     => join(' ', map { detokenize($_) } @{$_->{"keywords"}}),
            task_id      => $_->{"task_id"},
            has_pass     => $_->{"has_pass"}
        };
    }
}

##### Get the template
my $tpl = HTML::Template->new(filename => "webigator-demo-$UILANG.tpl", utf8 => 1);

$tpl->param(top_dir => $TOP_DIR);
$tpl->param(error   => $error);
$tpl->param(list    => \@list);

print $cgi->header({-charset => 'UTF-8', -pragma => 'no-cache', 'Cache-Control' => 'no-cache'});
print $tpl->output;
