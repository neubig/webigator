#!/usr/bin/perl -w

use strict;
use utf8;
use Encode;
use CGI;
use CGI::Carp qw(fatalsToBrowser);
use XML::RPC;
use Data::Dumper;
use Template;

$Carp::Verbose = 1;

binmode STDIN,  ":utf8";
binmode STDOUT, ":utf8";
binmode STDERR, ":utf8";

require "settings.pl";
require "functions.pl";

our $SERVER;
our $TOP_DIR;
our $ALLOW_TASK_ADD;

##### Get the connection to the server
my $url = "http://$SERVER/RPC2";
my $xmlrpc = XML::RPC->new($url, utf8_flag => 1);

##### Get the parameter list
my $cgi = new CGI;
my $params = {map{$_, Encode::decode("utf8", $cgi->Vars->{$_})} $cgi->Vars};
my $task_id       = int($params->{"task_id"});
my $task_keywords = $params->{"task_keywords"};
my $admin_pass    = $params->{"pass"};
my $user_pass     = $params->{"user_pass"};

my $error = "";
my $alert = "";
my $success = 0;

sub reset_keywords {
    my @keywords = @_;
    # TODO my $result = $xmlrpc->call("remove_keywords", {task_id => $task_id});
    my $result = 1;
    if($result != 1) {
        $alert = "キーワードの削除が失敗しました：".$result->{"faultString"};
    } else {
        for(@keywords) { 
            my $tok_keyword = tokenize($_); utf8::encode($tok_keyword);
            my $result = $xmlrpc->call("add_keyword", {id => -1, text => $tok_keyword, lab => 1, task_id => "$task_id"});
            if($result != 1) {
                $alert = "キーワード追加が失敗しました：".$result->{"faultString"};
            }
        }
    }
}

if($params->{"edit_task"}) {
    ##### Check pass
    my $result;
    eval { $result = $xmlrpc->call("check_admin_pass", {task_id => $task_id, pass => $admin_pass}); };
    if ($result != 1) {
        $error = "正しいパスワードを入力してください。";
    } else {
        # Get task keywords
        $xmlrpc->{tpp}->set(utf8_flag => 1);
        eval { $result = $xmlrpc->call("get_keywords", {task_id => $task_id}); };
        $xmlrpc->{tpp}->set(utf8_flag => 0);
        if($@) {
            $error = "サーバ${SERVER}への接続が失敗しました！作業ツールが動かない可能性が高いです。";
        } elsif(UNIVERSAL::isa($result,'HASH') and $result->{"faultString"}) {
            $error = "サーバ${SERVER}からのキーワード取得が失敗しました：".$result->{"faultString"};
        } else {
            my @keywords;
            $task_keywords = join(" ", map { detokenize($_) } @$result);
            $user_pass = $xmlrpc->call("get_user_pass", {task_id => $task_id, pass => $admin_pass});
        }
    }
}
##### If a new task has been submitted, add it
elsif($params->{"add_task"}) {
    if((!$task_keywords) or (!$admin_pass)) {
        $alert = "新しいタスクを作るためにキーワードと管理者パスワードを入れる必要があります。";
    } else {
        if(!$task_id) {
            if(!$ALLOW_TASK_ADD) {
                $alert = "タスクの追加が無効になっています。";
            } else {
                $task_id = $xmlrpc->call("add_task", {user_pass => $user_pass, admin_pass => $admin_pass});
            }
        } else {            
            my $result;
            eval { $result = $xmlrpc->call("check_admin_pass", {task_id => $task_id, pass => $admin_pass}); };
            if ($result != 1) {
                $error = "正しいパスワードを入力してください。";
            }
        }
        if ((UNIVERSAL::isa($task_id,'HASH') and $task_id->{"faultString"})) { 
            $alert = "タスクの追加に失敗しました：".$task_id->{"faultString"};
        } elsif(!$error && !$alert) {
            reset_keywords(split(/[ 　]+/, $task_keywords));
        }
        # success
        if (!$error && !$alert) {
            #print $cgi->redirect('webigator-run.cgi?task_id='.$task_id);
            $success = 1;
        }
    }
}

##### Get the template
my $tpl = HTML::Template->new(filename => 'webigator-task.tpl', utf8 => 1);

$tpl->param(top_dir => $TOP_DIR);
$tpl->param(error   => $error);
$tpl->param(alert   => $alert);
$tpl->param(success => $success);

$tpl->param(task_id        => $task_id);
$tpl->param(task_keywords  => $task_keywords);
$tpl->param(allow_task_add => ($ALLOW_TASK_ADD || $task_id) ? 1 : 0);
$tpl->param(admin_pass     => $admin_pass);
$tpl->param(user_pass      => $user_pass);

print $cgi->header({-charset => 'UTF-8', -pragma => 'no-cache', 'Cache-Control' => 'no-cache'});
print $tpl->output;
