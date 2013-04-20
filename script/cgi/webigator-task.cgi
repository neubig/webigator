#!/usr/bin/perl -w

use strict;
use utf8;
use Encode;
use CGI;
# use CGI::Carp qw(fatalsToBrowser);
use XML::RPC;
use Data::Dumper;
use Template;

$Carp::Verbose = 1;

binmode STDIN,  ":utf8";
binmode STDOUT, ":utf8";
binmode STDERR, ":utf8";

require "settings.pl";
require "functions.pl";

our $UILANG;
our $SERVER;
our $TOP_DIR;
our $ALLOW_TASK_ADD;

##### Localization messages
my %MSG;
if($UILANG eq "ja") {
    %MSG = (
        "wrong-password" => "正しいパスワードを入力してください。",
        "failed-connection" => "サーバ${SERVER}への接続が失敗しました。",
        "failed-label" => "ラベルの投稿に失敗しました：",
        "failed-keyword" => "サーバ${SERVER}からのキーワード取得が失敗しました：",
        "failed-retrieval" => "ツイートの獲得に失敗しました：",
        "failed-task-add" => "タスクの追加に失敗しました：",
        "required-fields" => "全ての必須項目を入力してください。",
        "no-task-add" => "タスクの追加が無効になっています。",
    );
} else {
    %MSG = (
        "wrong-password" => "The password entered was incorrect.",
        "failed-connection" => "Could not connect to server ${SERVER}.",
        "failed-label" => "Could not submit label: ",
        "failed-keyword" => "Could not retrieve keywords from server ${SERVER}: ",
        "failed-retrieval" => "Could not retrieve information.",
        "failed-task-add" => "Could not add a task.",
        "required-fields" => "Please fill in all the required fields.",
        "no-task-add" => "Adding tasks is disabled",
    );
}

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
        $alert = $MSG{"failed-keyword"}.$result->{"faultString"};
    } else {
        for(@keywords) { 
            my $tok_keyword = tokenize($_); utf8::encode($tok_keyword);
            my $result = $xmlrpc->call("add_keyword", {id => -1, text => $tok_keyword, lab => 1, task_id => "$task_id"});
            if($result != 1) {
                $alert = $MSG{"failed-keyword"}.$result->{"faultString"};
            }
        }
    }
}

if($params->{"edit_task"}) {
    ##### Check pass
    my $result;
    eval { $result = $xmlrpc->call("check_admin_pass", {task_id => $task_id, pass => $admin_pass}); };
    if ($result != 1) {
        $error = $MSG{"wrong-password"};
    } else {
        # Get task keywords
        $xmlrpc->{tpp}->set(utf8_flag => 1);
        eval { $result = $xmlrpc->call("get_keywords", {task_id => $task_id}); };
        $xmlrpc->{tpp}->set(utf8_flag => 0);
        if($@) {
            $error = $MSG{"failed-connection"};
        } elsif(UNIVERSAL::isa($result,'HASH') and $result->{"faultString"}) {
            $error = $MSG{"failed-keyword"}.$result->{"faultString"};
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
        $alert = $MSG{"required-fields"};
    } else {
        if(!$task_id) {
            if(!$ALLOW_TASK_ADD) {
                $alert = $MSG{"no-task-add"};
            } else {
                $task_id = $xmlrpc->call("add_task", {user_pass => $user_pass, admin_pass => $admin_pass});
            }
        } else {            
            my $result;
            eval { $result = $xmlrpc->call("check_admin_pass", {task_id => $task_id, pass => $admin_pass}); };
            if ($result != 1) {
                $error = $MSG{"wrong-password"};
            }
        }
        if ((UNIVERSAL::isa($task_id,'HASH') and $task_id->{"faultString"})) { 
            $alert = $MSG{"failed-task-add"}.$task_id->{"faultString"};
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
my $tpl = HTML::Template->new(filename => "webigator-task-$UILANG.tpl", utf8 => 1);

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
