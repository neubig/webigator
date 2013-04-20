#!/usr/bin/perl -w

use strict;
use utf8;
use Encode;
use CGI;
# use CGI::Carp qw(fatalsToBrowser);
use XML::RPC;
use Data::Dumper;
use Template;

binmode STDIN,  ":utf8";
binmode STDOUT, ":utf8";
binmode STDERR, ":utf8";

require "settings.pl";
require "functions.pl";

our $UILANG;
our $SERVER;
our $TWEET_COUNT;
our $TOP_DIR;

##### Localization messages
my %MSG;
if($UILANG eq "ja") {
    %MSG = (
        "wrong-password" => "正しいパスワードを入力してください。",
        "failed-connection" => "サーバ${SERVER}への接続が失敗しました。",
        "failed-label" => "ラベルの投稿に失敗しました：",
        "failed-keyword" => "サーバ${SERVER}からのキーワード取得が失敗しました：",
        "failed-retrieval" => "ツイートの獲得に失敗しました：",
    );
} else {
    %MSG = (
        "wrong-password" => "The password entered was incorrect.",
        "failed-connection" => "Could not connect to server ${SERVER}.",
        "failed-label" => "Could not submit label: ",
        "failed-keyword" => "Could not retrieve keywords from server ${SERVER}: ",
        "failed-retrieval" => "Could not retrieve information.",
    );
}

##### Get the connection to the server
my $url = "http://$SERVER/RPC2";
my $xmlrpc = XML::RPC->new($url, utf8_flag => 1);

##### Get the parameter list
my $cgi = new CGI;
my $params = {map{$_, Encode::decode("utf8", $cgi->Vars->{$_})} $cgi->Vars};
my $task_id = int($params->{"task_id"});
my $user_pass = $params->{"pass"};

my $error = "";

my @keywords;
my @list;

##### Check pass
my $result;
eval { $result = $xmlrpc->call("check_user_pass", {task_id => $task_id, pass => $user_pass}); };
if ($result != 1) {
    $error = $MSG{"wrong-password"};
}

##### Post labels
if((!$error) and ($params->{"post_labels"})) {
    foreach my $i (0 .. $TWEET_COUNT-1) {
        next if((not $params->{"id$i"}) or (not $params->{"text$i"}) or (not $params->{"lab$i"}));
        my $label = 2;
        if ($params->{"lab$i"} eq "y") { $label = 1; }
        elsif ($params->{"lab$i"} eq "n") { $label = 0; }
        if($label != 2) {
            my $result;
            my $text = tokenize($params->{"text$i"}); utf8::encode($text);
            eval { $result = $xmlrpc->call("add_labeled", {text => $text, id => $params->{"id$i"}, lab => $label, task_id => $task_id}); };
            if ($@) { 
                $error = $MSG{"failed-connection"}; last;
            } elsif ((UNIVERSAL::isa($result,'HASH') and $result->{"faultString"})) { 
                $error = $MSG{"failed-label"}.$result->{"faultString"}; last;
            }
        }
    }
}

##### Get keywords
if (!$error) {
    my $result;
    eval { $result = $xmlrpc->call("get_keywords", {task_id => $task_id}); };
    if($@) {
        $error = $MSG{"failed-connection"};
    } elsif(UNIVERSAL::isa($result,'HASH') and $result->{"faultString"}) {
        $error = $MSG{"failed-keyword"}.$result->{"faultString"};
    } else {
        push @keywords, {keyword => detokenize($_)} for(@$result);
    }
}

##### Get the list of tweets
if (!$error && @keywords) {
    foreach my $i (0 .. $TWEET_COUNT-1) {
        my $result;
        eval { $result = $xmlrpc->call("pop_best", {task_id => $task_id}); };
        if ($@) { 
            $error = $MSG{"failed-connection"};
        } elsif (!$result) {
            last;
        } elsif ((UNIVERSAL::isa($result,'HASH') and $result->{"faultString"})) { 
            $error = $MSG{"failed-retrieval"}.$result->{"faultString"}; last;
        }
        
        push @list, {
            index => $i,
            text  => detokenize($result->{"text"}),
            id    => $result->{"id"},
        };
    }
}

##### Get the template
my $tpl = HTML::Template->new(filename => "webigator-run-$UILANG.tpl", utf8 => 1);

$tpl->param(top_dir  => $TOP_DIR);
$tpl->param(error    => $error);

$tpl->param(task_id  => $task_id);
$tpl->param(pass     => $user_pass);
$tpl->param(keywords => \@keywords);
$tpl->param(list     => \@list);

print $cgi->header({-charset => 'UTF-8', -pragma => 'no-cache', 'Cache-Control' => 'no-cache'});
print $tpl->output;
