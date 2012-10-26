#!/usr/bin/perl -w

use strict;
use utf8;
use Encode;
use CGI;
use CGI::Carp qw(fatalsToBrowser);
use XML::RPC;
use Data::Dumper;
use Template;

binmode STDIN,  ":utf8";
binmode STDOUT, ":utf8";
binmode STDERR, ":utf8";

require "settings.pl";
require "functions.pl";

our $SERVER;
our $TOP_DIR;

##### Get the connection to the server
my $url = "http://$SERVER/RPC2";
my $xmlrpc = XML::RPC->new($url, utf8_flag => 1);

##### Get the parameter list
my $cgi = new CGI;
my $params = {map{$_, Encode::decode("utf8", $cgi->Vars->{$_})} $cgi->Vars};
my $task_id       = int($params->{"task_id"});
my $task_keywords = $params->{"task_keywords"};
my $admin_pass    = $params->{"admin_pass"};
my $user_pass     = $params->{"user_pass"};

my $error = "";
my $alert = "";
my $success = 0;

##### If a new task has been submitted, add it
if($params->{"add_task"}) {
    if((!$task_keywords) or (!$admin_pass)) {
        $alert = "新しいタスクを作るためにキーワードと管理者パスワードを入れる必要があります。";
    } else {
        $task_id = $xmlrpc->call("add_task", {user_pass => $user_pass, admin_pass => $admin_pass});
        if ((UNIVERSAL::isa($task_id,'HASH') and $task_id->{"faultString"})) { 
            $alert = "タスクの追加に失敗しました：".$task_id->{"faultString"};
        } else {
            my @keywords = split(/[ 　]+/, $task_keywords);
            for(@keywords) { 
                my $tok_keyword = tokenize($_); utf8::encode($tok_keyword);
                my $result = $xmlrpc->call("add_keyword", {id => -1, text => $tok_keyword, lab => 1, task_id => $task_id});
                if($result != 1) {
                    $alert = "キーワード追加が失敗しました：".$result->{"faultString"};
                }
            }
        }
        # success
        if (!$error && !$alert) {
            #print $cgi->redirect('webigator-run.cgi?task_id='.$task_id);
            $success = 1;
        }
    }
}

##### Check pass
#if (!$error) {
#    if (0) {  # check
#        $error = '正しいパスワードを入力してください。';
#    }
#}

##### Get the template
my $tpl = HTML::Template->new(filename => 'webigator-task.tpl', utf8 => 1);

$tpl->param(top_dir => $TOP_DIR);
$tpl->param(error   => $error);
$tpl->param(alert   => $alert);
$tpl->param(success => $success);

$tpl->param(task_id       => $task_id);
$tpl->param(task_keywords => $params->{"task_keywords"});
$tpl->param(admin_pass    => $params->{"admin_pass"});
$tpl->param(user_pass     => $params->{"user_pass"});

print $cgi->header({-charset => 'UTF-8', -pragma => 'no-cache', 'Cache-Control' => 'no-cache'});
print $tpl->output;
