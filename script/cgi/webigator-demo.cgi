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
our $TOP_DIR;

##### Get the connection to the server
my $url = "http://$SERVER/RPC2";
my $xmlrpc = XML::RPC->new($url);

##### Get the parameter list
my $cgi = new CGI;
my $params = $cgi->Vars;
my $error = "";
my $result;

##### Get the list of tasks
my $task_table;
$xmlrpc->{tpp}->set( utf8_flag => 1 ); # This is a hack so UTF works
eval { $result = $xmlrpc->call("get_tasks"); };
$xmlrpc->{tpp}->set( utf8_flag => 0 ); # This is a hack so UTF works
my @arr;
if($@) {
    $error = "サーバ${SERVER}への接続が失敗しました！サーバを立ち上げる必要があります。";
} elsif(UNIVERSAL::isa($result,'HASH') and $result->{"faultString"}) {
    $error = "サーバ${SERVER}からのタスク取得が失敗しました：".$result->{"faultString"};
} else {
    for(@$result) {
        my @keywords = map { detokenize($_) } @{$_->{"keywords"}};
        my $task_id = $_->{"task_id"};
        my $has_password = $_->{"has_password"};
        $task_table .= "
          <tr class=\"tl_tr\">
            <form action=\"webigator-run.cgi\" method=\"post\">
            <td>$task_id<input type=\"hidden\" name=\"task_id\" value=\"$task_id\" /></td>
            <td class=\"tl_td_tweet\"><span>".join("&nbsp;&nbsp;",@keywords)."</span></td>
            <td>".($has_password?"<font color=\"660000\">あり</font>":"<font color=\"006600\">なし</font>")."</td>
            <td><input type=\"submit\" value=\"参加\" class=\"btn\"/>
            </form>
          </tr>
        ";
    }
}


##### Print the page
print $cgi->header(-charset=>"utf-8");
print "
<!doctype html>
<html xmlns=\"http://www.w3.org/1999/xhtml\" xml:lang=\"ja-JP\" lang=\"ja-JP\">
<head>
<meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\" />
<title>webigator タスク一覧</title>
<link href=\"$TOP_DIR/css/main.css\" rel=\"stylesheet\" type=\"text/css\" />
<link href=\"$TOP_DIR/css/webigator.css\" rel=\"stylesheet\" type=\"text/css\" />
<script type=\"text/javascript\" src=\"https://ajax.googleapis.com/ajax/libs/jquery/1.8.2/jquery.min.js\"></script>
<script type=\"text/javascript\" src=\"$TOP_DIR/js/jquery.cookie.js\"></script>
<script type=\"text/javascript\" src=\"$TOP_DIR/js/webigator.js\"></script>
</head>

<body>

<div class=\"container\">
  <div class=\"header\">
    <h1>webigator<span class=\"demo\">タスク一覧</span></h1>
    <!-- end .header --></div>
    
    <div class=\"memo\">
      <div class=\"memo_tag\"><a href=\"#\" onClick=\"return toggleMemo()\">説明</a></div>
      <div class=\"memo_text radius_top radius_bottom\">
        これはWebからの情報集約を手伝うシステム、<a href='http://www.github.com/neubig/webigator' target=\"_blank\">webigator</a>のデモです。<br/>
        各集約タスクは１種類の情報を対象としています。
        例えば、「奈良県のイベント情報」のタスクなら、システムが奈良県のイベントに関するテキスト（主にTwitter上のツイート）を提示してくれて、これを手作業で整理し、Webサイト上の一覧を作ることができます。<br /><br />
        ・<b>新しいタスクを作りたい</b>場合は必要な情報を入力し、「タスク登録」ボタンを押してください。<br />
        ・<b>タスク一覧に載っているタスクに参加したい</b>場合は「参加」ボタンを押してください。
        <br /><br />[ <a href=\"#\" onClick=\"return toggleMemo()\">説明を閉じる</a> ]
      </div>
    </div>
    
  <div class=\"content\">";
print "<p><font color=red>$error</font></p>" if $error;
print "  <div class=\"keyword\">
      <form action=\"webigator-run.cgi\" method=\"post\">
      <div class=\"keyword_new radius_top\">
        <table>
            <tr><th>検索キーワード<br/>(空白で区切った複数可)</th><td><td class=\"box\"><input type=\"text\" name=\"task_keywords\" /></td></tr>
            <tr><th>管理者パスワード(必須)</th><td><td class=\"box\"><input type=\"text\" name=\"admin_pass\" /></td></tr>
            <tr><th>作業者パスワード(任意)</th><td><td class=\"box\"><input type=\"text\" name=\"user_pass\" /></td></tr>
            <tr><td colspan=3 align=center><input type=\"submit\" value=\"タスク登録\" class=\"btn\" name=\"add_task\" /></td></tr>
          </table>
      </div>
      </form>
    </div>

    <div class=\"tweet\">

      以降のタスクは現在進行中です。
      手伝いたいものにクリックし、必要に応じてパスワードを入力してください。

      <div class=\"timeline\">
        <table width=\"100%\">
          <tr>
            <th>タスクID</th>
            <th>キーワード</th>
            <th>パスワード</th>
            <th>&nbsp;</th>
          </tr>
          $task_table
        </table>
      </div>

    </div>

    <!-- end .content --></div>
  <div class=\"footer\">
    <!-- end .footer --></div>
  <!-- end .container --></div>
</body>
</html>
";
#           <tr class=\"tl_tr\">
#             <form action=\"webigator-run.cgi\" method=\"post\">
#             <td>12382101<input type=\"hidden\" name=\"task_id\" value=\"12382101\" /></td>
#             <td class=\"tl_td_tweet\"><span>給水 炊き出し 充電</span></td>
#             <td><font color=\"660000\"><input type=\"text\" name=\"task_password\" /></font></td>
#             <td><input type=\"submit\" value=\"参加\" class=\"btn\"/>
#             </form>
#           </tr>
#           <tr class=\"tl_tr\">
#             <form action=\"webigator-run.cgi\" method=\"post\">
#             <td>491991293<input type=\"hidden\" name=\"task_id\" value=\"491991293\" /></td>
#             <td class=\"tl_td_tweet\"><span>安否 確認 できました</span></td>
#             <td><font color=\"006600\">なし</font></td>
#             <td><input type=\"submit\" value=\"参加\" class=\"btn\"/>
#             </form>
#           </tr>
exit;
