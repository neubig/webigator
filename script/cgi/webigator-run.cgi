#!/usr/bin/perl

use strict;
use utf8;
use CGI;
use Getopt::Long;
use XML::RPC;
use Data::Dumper;
use List::Util qw(sum min max shuffle);
binmode STDIN, ":utf8";
binmode STDOUT, ":utf8";
binmode STDERR, ":utf8";

require "settings.pl";
require "functions.pl";
our $SERVER;
our $TWEET_COUNT;
our $TOP_DIR;

##### Get the connection to the server
my $url = "http://$SERVER/RPC2";
my $xmlrpc = XML::RPC->new($url);

##### Get the parameter list
my $cgi = new CGI;
my $params = $cgi->Vars;
my $task_id = int($params->{"task_id"});
my $added_keyword_message;
my $tweet_table = "<input type=\"submit\" value=\"ツイートを表示\" name=\"view_tweets\" class=\"btn\" />";
my $error = "";

##### Get keywords #####
sub get_keywords() {
    my $result;
    $xmlrpc->{tpp}->set( utf8_flag => 1 ); # This is a hack so UTF works
    eval { $result = $xmlrpc->call("get_keywords", {task_id => $task_id}); };
    $xmlrpc->{tpp}->set( utf8_flag => 0 ); # This is a hack so UTF works
    my @arr;
    if($@) {
        $error = "サーバ${SERVER}への接続が失敗しました！作業ツールが動かない可能性が高いです。";
    } elsif(UNIVERSAL::isa($result,'HASH') and $result->{"faultString"}) {
        $error = "サーバ${SERVER}からのキーワード取得が失敗しました：".$result->{"faultString"};
    } else {
        @arr = map { detokenize($_) } @$result;
        push @arr, "キーワードはまだ選ばれていません！" if not @arr;
    }
    return @arr;
}

##### If a new task has been submitted, add it
if($params->{"add_task"}) {
    my $task_keywords = $params->{"task_keywords"};
    my $admin_pass = $params->{"admin_pass"};
    if((!$task_keywords) or (!$admin_pass) or ($task_id)) {
        $error = "新しいタスクを作るためにキーワードと管理者パスワードを入れる必要があります。";
    } else {
        $task_id = $xmlrpc->call("add_task");
        if ((UNIVERSAL::isa($task_id,'HASH') and $task_id->{"faultString"})) { 
            $error = "タスクの追加に失敗しました：".$task_id->{"faultString"};
        } else {
            utf8::decode($task_keywords); 
            my @keywords = split(/ /, $task_keywords);
            for(@keywords) { 
                my $tok_keyword = tokenize($_); utf8::encode($tok_keyword);
                my $result = $xmlrpc->call("add_keyword", {id => -1, text => $tok_keyword, lab => 1, task_id => $task_id});
                if($result != 1) {
                    $error = "キーワード追加が失敗しました：".$result->{"faultString"};
                } else {
                    $added_keyword_message .= "<p><font color=green>「$_」をキーワードとして追加しました。</font></p>";
                }
            }
        }
    }
}

##### If keywords have been submitted, add them
elsif($params->{"keyword"}) {
    my $keyword = $params->{"keyword"};  utf8::decode($keyword);
    my $tok_keyword = tokenize($keyword); utf8::encode($tok_keyword);
    my $result = $xmlrpc->call("add_keyword", {id => -1, text => $tok_keyword, lab => 1, task_id => $task_id});
    if($result != 1) {
        $added_keyword_message = "<p><font color=red>キーワード追加が失敗しました：".$result->{"faultString"}."</font></p>";
    } else {
        $added_keyword_message = "<p><font color=green>「$keyword」をキーワードとして追加しました。</font></p>";
    }
}

##### If we want to view tweets, download them
elsif($params->{"view_tweets"}) {
    $xmlrpc->{tpp}->set( utf8_flag => 1 ); # This is a hack so UTF works
    my $tweet_rows;
    foreach my $i (0 .. $TWEET_COUNT-1) {
        my $result;
        eval { $result = $xmlrpc->call("pop_best", {task_id => $task_id}); };
        if ($@) { 
            $error = "サーバ${SERVER}への接続が失敗しました。"; last;
        } elsif (!$result) {
            last;
        } elsif ((UNIVERSAL::isa($result,'HASH') and $result->{"faultString"})) { 
            $error = "ツイートの獲得に失敗しました：".$result->{"faultString"}; last;
        }
        my $esc_text1 = $result->{"text"}; $esc_text1 = detokenize($esc_text1); $esc_text1 =~ s/</&lt;/g; $esc_text1 =~ s/>/&gt;/g; $esc_text1 =~ s/\\n/<br\/>/g;
        my $esc_text2 = $result->{"text"}; $esc_text2 =~ s/"/\\"/g;
        my $id = $result->{"id"};
        $tweet_rows .= "
          <tr class=\"tl_tr\">
            <td class=\"tl_td_label\">
              <ul>
                <li class=\"label_y_off\"><a href=\"#\" id=\"image_y_$i\"></a></li>
                <li class=\"label_n_off\"><a href=\"#\" id=\"image_n_$i\"></a></li>
              </ul>
              <input type=\"hidden\" id=\"lab$i\" name=\"lab$i\" value=\"?\" />
            </td>
            <td class=\"tl_td_tweet\">
              <span>$esc_text1</span>
              <input type=\"hidden\" name=\"text$i\" value=\"$esc_text2\" />
            </td>
            <td>$id<input type=\"hidden\" name=\"id$i\" value=\"$id\" /></td>
          </tr>
         ";
    }
    $xmlrpc->{tpp}->set( utf8_flag => 0 ); # This is a hack so UTF works
    if(!$tweet_rows) {
        $error = "現在、新しく閲覧できるデータがありません。このキーワードに関して新たに取得できるテキストがなかったか、情報を取得するプログラムが動いていない可能性があります。";
    } elsif(!$error) {
        $tweet_table = " 
        <div class=\"timeline\">
            <table width=\"100%\">
            <tr><th class=\"tl_td_label\">ラベル</th><th>テキスト</th><th>ツイートID</th></tr>
            $tweet_rows
            </table>
            <input type=submit name=\"post_labels\" value=\"ラベルを投稿\" class=\"btn\" />
        </div>
        ";
    }
} elsif($params->{"post_labels"}) {
    my @colors = ("red", "green", "black");
    my $tweet_rows;
    foreach my $i (0 .. $TWEET_COUNT-1) {
        next if((not $params->{"id$i"}) or (not $params->{"text$i"}) or (not $params->{"lab$i"}));
        my $label = 2;
        if ($params->{"lab$i"} eq "y") { $label = 1; }
        elsif ($params->{"lab$i"} eq "n") { $label = 0; }
        if($label != 2) {
            my $result = $xmlrpc->call("add_labeled", {text => $params->{"text$i"}, id => $params->{"id$i"}, lab => $label, task_id => $task_id});
            if ($@) { 
                $error = "サーバ${SERVER}への接続が失敗しました。"; last;
            } elsif ((UNIVERSAL::isa($result,'HASH') and $result->{"faultString"})) { 
                $error = "ツイートの獲得に失敗しました：".$result->{"faultString"}; last;
            }
        }
        my $esc_text1 = $params->{"text$i"}; utf8::decode($esc_text1); $esc_text1 = detokenize($esc_text1); $esc_text1 =~ s/</&lt;/g; $esc_text1 =~ s/>/&gt;/g; $esc_text1 =~ s/\\n/<br\/>/g;
        my $id = $params->{"id$i"};  $id =~ s/</&lt;/g; $id =~ s/>/&gt;/g;
        $tweet_rows .= "
          <tr class=\"tl_tr\">
            <td class=\"tl_td_tweet\">
              <span><font color=$colors[$label]>$esc_text1</font></span>
            </td>
            <td>$id</td>
          </tr>
         ";
    }
    if(!$tweet_rows) {
        $error = "現在、新しく閲覧できるデータがありません。このキーワードに関して新たに取得できるテキストがなかったか、情報を取得するプログラムが動いていない可能性があります。";
    } elsif(!$error) {
        $tweet_table = " 
        <div class=\"timeline\">
            <table width=\"100%\">
            <tr><th>テキスト</th><th>ツイートID</th></tr>
            $tweet_rows
            </table>
            <input type=\"submit\" value=\"ツイートを表示\" name=\"view_tweets\" class=\"btn\" />
        </div>
        ";
    }
}

##### Print the web page #####
print $cgi->header(-charset=>"utf-8");
print "
<!doctype html>
<html xmlns=\"http://www.w3.org/1999/xhtml\" xml:lang=\"ja-JP\" lang=\"ja-JP\">
<head>
<meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\" />
<title>webigator 作業ツール</title>
<link href=\"$TOP_DIR/css/main.css\" rel=\"stylesheet\" type=\"text/css\" />
<link href=\"$TOP_DIR/css/webigator.css\" rel=\"stylesheet\" type=\"text/css\" />
<script type=\"text/javascript\" src=\"https://ajax.googleapis.com/ajax/libs/jquery/1.8.2/jquery.min.js\"></script>
<script type=\"text/javascript\" src=\"$TOP_DIR/js/jquery.cookie.js\"></script>
<script type=\"text/javascript\" src=\"$TOP_DIR/js/webigator.js\"></script>
</head>

<body>

<div class=\"container\">
  <div class=\"header\">
    <h1>webigator<span class=\"demo\">作業ツール</span> <a href=\"webigator-demo.cgi\"><span class=\"demo\">(一覧に戻る)</span></a></h1>
    <!-- end .header --></div>
    
    <div class=\"memo\">
      <div class=\"memo_tag\"><a href=\"#\" onClick=\"return toggleMemo()\">説明</a></div>
      <div class=\"memo_text radius_top radius_bottom\">
        <a href='http://www.github.com/neubig/webigator' target=\"_blank\">webigator</a>を使って有用なツイートを発見するツールです。<br /><br />
        作業として：<br />
        <ol>
        <li><b>キーワード入力：</b>探したい情報に関連するキーワードを入れます。Webページやツイッターを探す場合と同じような形で問題ないです。</li>
        <li><b>ツイートの発見：</b>ツイートの中で、有用なものは\"＋\"のラベルを、有用でないものは\"－\"のラベルを選択してください。判断のつかないものは、そのままでかまいません。</li>
        </ol><br />
        注：現在は最初の方に有用でないツイートばかりがでます。この問題は改善予定ですが、今は有用なものが一個でも出るまで我慢すれば問題が改善されます。
        <br /><br />[ <a href=\"#\" onClick=\"return toggleMemo()\">説明を閉じる</a> ]
      </div>
    </div>
    
  <div class=\"content\">";
print "<p><font color=red>$error</font></p>" if $error;
print "    <div class=\"keyword\">
      <form action=\"webigator-run.cgi\" method=\"post\">
      <input type=\"hidden\" name=\"task_id\" value=\"$task_id\">
      <div class=\"keyword_new radius_top\">
        <table><tr><th>検索キーワード</th><td>
          <table><tr><td class=\"box\"><input type=\"text\" name=\"keyword\" /></td><td><input type=\"submit\" value=\"登録\" class=\"btn\" /></td></tr>
          </table>
        </td></tr></table>
      </div>
      <div class=\"keyword_lst radius_bottom\">
        <table><tr><th>現在のキーワード</th><td><ul>
    ".join("", map { "<li>$_</li>" } get_keywords())."
        </ul><br class=\"clearfloat\" /></td></tr></table>
      </div>
      </form>
    </div>
    $added_keyword_message
    <div class=\"tweet\">
    <h2>ツイートの発見：</h2>
      有用なものは\"＋\"のラベルを、有用でないものは\"－\"のラベルを選択してください。<br />
      判断のつかないものは、そのままで構いません。
      <form action=\"webigator-run.cgi\" method=post>
      <input type=\"hidden\" name=\"task_id\" value=\"$task_id\">
      $tweet_table
      </form>
    </div>

    <!-- end .content --></div>
  <div class=\"footer\">
    <!-- end .footer --></div>
  <!-- end .container --></div>
</body>
</html>
";
# print "<pre>".Dumper($params)."</pre>";
exit;
