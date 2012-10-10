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
our $SERVER;
our $TWEET_COUNT;
our $TOP_DIR;

##### Get the connection to the server
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

##### Get keywords #####
sub get_keywords() {
    my $result;
    $xmlrpc->{tpp}->set( utf8_flag => 1 ); # This is a hack so UTF works
    eval { $result = $xmlrpc->call("get_keywords"); };
    $xmlrpc->{tpp}->set( utf8_flag => 0 ); # This is a hack so UTF works
    return @{["<font color=red>サーバ${SERVER}への接続が失敗しました！デモが動かない可能性が高いです。</font>"]} if ($@);
    return @{[$result->{"faultString"}]} if (UNIVERSAL::isa($result,'HASH') and $result->{"faultString"});
    my @arr = map { detokenize($_) } @$result;
    push @arr, "キーワードはまだ選ばれていません！" if not @arr;
    return @arr;
}

##### Get the parameter list
my $cgi = new CGI;
my $params = $cgi->Vars;

##### If keywords have been submitted, add them
my $added_keyword_message;
if($params->{"keyword"}) {
    my $keyword = $params->{"keyword"};  utf8::decode($keyword);
    my $tok_keyword = tokenize($keyword); utf8::encode($tok_keyword);
    my $result = $xmlrpc->call("add_keyword", {id => -1, text => $tok_keyword, lab => 1});
    if($result != 1) {
        $added_keyword_message = "<p><font color=red>キーワード追加が失敗しました：".$result->{"faultString"}."</font></p>";
    } else {
        $added_keyword_message = "<p><font color=green>「$keyword」をキーワードとして追加しました。</font></p>";
    }
}

##### If we want to view tweets, download them
my $tweet_table = "<input type=\"submit\" value=\"ツイートを表示\" class=\"btn\" />";
if($params->{"view_tweets"}) {
    $tweet_table = "";
    my $err = "";
    $xmlrpc->{tpp}->set( utf8_flag => 1 ); # This is a hack so UTF works
    foreach my $i (0 .. $TWEET_COUNT-1) {
        my $result;
        eval { $result = $xmlrpc->call("pop_best"); };
        if ($@) { 
            $err = "サーバ${SERVER}への接続が失敗しました。"; last;
        } elsif (!$result) {
            last;
        } elsif ((UNIVERSAL::isa($result,'HASH') and $result->{"faultString"})) { 
            $err = "ツイートの獲得に失敗しました：".$result->{"faultString"}; last;
        }
        my $esc_text1 = $result->{"text"}; $esc_text1 = detokenize($esc_text1); $esc_text1 =~ s/</&lt;/g; $esc_text1 =~ s/>/&gt;/g; $esc_text1 =~ s/\\n/<br\/>/g;
        my $esc_text2 = $result->{"text"}; $esc_text2 =~ s/"/\\"/g;
        my $id = $result->{"id"};
        $tweet_table .= "
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
    $err = "現在、読み込むツィートはありません。" if ($tweet_table eq "");
    if($err) {
        $tweet_table = "<p><font color=red>$err</font></p><input type=submit name=view_tweets value=\"ツイートを表示\"/>";
    } else {
        $tweet_table = "<table border=1 cellspacing=0 width=800>
            <tr>
                <th class=\"tl_td_label\">ラベル</th>
                <th>テキスト</th>
                <th>ツイートID</th>
            </tr>
            $tweet_table
            </table>
            ";
    }
} elsif($params->{"post_labels"}) {
    my @colors = ("red", "green", "black");
    $tweet_table = "";
    my $err;
    foreach my $i (0 .. $TWEET_COUNT-1) {
        next if((not $params->{"id$i"}) or (not $params->{"text$i"}) or (not $params->{"lab$i"}));
        my $label = 2;
        if ($params->{"lab$i"} eq "y") { $label = 1; }
        elsif ($params->{"lab$i"} eq "n") { $label = 0; }
        if($label != 2) {
            my $result = $xmlrpc->call("add_labeled", {text => $params->{"text$i"}, id => $params->{"id$i"}, lab => $label});
            if ($@) { 
                $err = "サーバ${SERVER}への接続が失敗しました。"; last;
            } elsif ((UNIVERSAL::isa($result,'HASH') and $result->{"faultString"})) { 
                $err = "ツイートの獲得に失敗しました：".$result->{"faultString"}; last;
            }
        }
        my $esc_text1 = $params->{"text$i"}; utf8::decode($esc_text1); $esc_text1 = detokenize($esc_text1); $esc_text1 =~ s/</&lt;/g; $esc_text1 =~ s/>/&gt;/g; $esc_text1 =~ s/\\n/<br\/>/g;
        my $id = $params->{"id$i"};  $id =~ s/</&lt;/g; $id =~ s/>/&gt;/g;
        my @arr; $arr[3] = "<font color=$colors[$label]>$esc_text1</font>"; $arr[$label] = "X"; $arr[4] = $id;
        $tweet_table .= "<tr>".join("", map { "<td>$_</td>" } @arr)."\n";
    }
    if($err) {
        $tweet_table = "<p><font color=red>$err</font></p><input type=submit name=view_tweets value=\"ツイートを表示\"/>";
    } else {
        $tweet_table = "<table border=1 cellspacing=0 width=800><tr><td>誤</td><td>正</td><td>??</td><td>テキスト</td><td>ID</td></tr>$tweet_table</table>
                        <input type=submit name=view_tweets value=\"ツイートを表示\"/>";
    }
}

##### Print the web page #####
print $cgi->header(-charset=>"utf-8");
print "
<!doctype html>
<html xmlns=\"http://www.w3.org/1999/xhtml\" xml:lang=\"ja-JP\" lang=\"ja-JP\">
<head>
<meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\" />
<title>webigator デモ</title>
<link href=\"$TOP_DIR/css/main.css\" rel=\"stylesheet\" type=\"text/css\" />
<link href=\"$TOP_DIR/css/webigator.css\" rel=\"stylesheet\" type=\"text/css\" />
<script type=\"text/javascript\" src=\"https://ajax.googleapis.com/ajax/libs/jquery/1.8.2/jquery.min.js\"></script>
<script type=\"text/javascript\" src=\"$TOP_DIR/js/jquery.cookie.js\"></script>
<script type=\"text/javascript\" src=\"$TOP_DIR/js/webigator.js\"></script>
</head>

<body>

<div class=\"container\">
  <div class=\"header\">
    <h1>webigator<span class=\"demo\">デモ</span></h1>
    <!-- end .header --></div>
    
    <div class=\"memo\">
      <div class=\"memo_tag\"><a href=\"#\" onClick=\"return toggleMemo()\">説明</a></div>
      <div class=\"memo_text radius_top radius_bottom\">
        <a href='http://www.github.com/neubig/webigator' target=\"_blank\">webigator</a>を使って東日本大震災後のツイートから有用なツイートを見つけるデモ＋実験です。<br />
        このページを使って、<a href=\"https://docs.google.com/spreadsheet/ccc?key=0Alj_-K_ClFGldGlaR1BsZEsxNHRtQnBlUXBLMFQ2RWc#gid=0\" target=\"_blank\">物資提供＋避難所情報一覧</a>を埋めていただければ幸いです。<br /><br />
        作業として：<br />
        <ol>
        <li><b>キーワード入力：</b>探したい情報に関連するキーワードを入れます。Webページやツイッターを探す場合と同じような形で問題ないです。</li>
        <li><b>ツイートの発見：</b>ツイートの中で、有用なものは\"＋\"のラベルを、有用でないものは\"－\"のラベルを選択してください。判断のつかないものは、そのままでかまいません。</li>
        </ol><br />
        注：現在は最初の方に有用でないツイートばかりがでます。この問題は改善予定ですが、今は有用なものが一個でも出るまで我慢すれば問題が改善されます。
        <br /><br />[ <a href=\"#\" onClick=\"return toggleMemo()\">説明を閉じる</a> ]
      </div>
    </div>
    
  <div class=\"content\">
    
    <div class=\"keyword\">
      <form action=\"webigator-demo.cgi\" method=\"post\">
      <div class=\"keyword_new radius_top\">
        <table><tr><th>検索キーワード</th><td>
          <table><tr><td class=\"box\"><input type=\"text\" name=\"keyword\" /></td><td><input type=\"submit\" value=\"登録\" class=\"btn\" /></td></tr>
          </table>
        </td></tr></table>
      </div>
      <div class=\"keyword_lst radius_bottom\">
        <table><tr><th>現在のキーワード</th><td><ul>
    ".join("、", map { "<li>$_</li>" } get_keywords())."
        </ul><br class=\"clearfloat\" /></td></tr></table>
      </div>
      </form>
    </div>
    $added_keyword_message
    
    <div class=\"tweet\">

      有用なものは\"＋\"のラベルを、有用でないものは\"－\"のラベルを選択してください。<br />
      判断のつかないものは、そのままでかまいません。
      <div class=\"timeline\">
        <form action=\"webigator-demo.cgi\" method=\"post\">
        <table width=\"100%\">
          <tr>
            <th class=\"tl_td_label\">ラベル</th>
            <th>テキスト</th>
            <th>ツイートID</th>
          </tr>
          <tr class=\"tl_tr\">
            <td class=\"tl_td_label\">
              <ul>
                <li class=\"label_y_off\"><a href=\"#\" id=\"image_y_0\"></a></li>
                <li class=\"label_n_off\"><a href=\"#\" id=\"image_n_0\"></a></li>
              </ul>
              <input type=\"hidden\" id=\"lab0\" name=\"lab0\" value=\"?\" />
            </td>

    </p>
    <!-- <form action=\"webigator-demo.cgi\" method=post>
    <table border=1 cellspacing=0>
    <tr><td colspan=2 align=center><b>キーワード追加フォーム</b></td></tr>
    <tr><td><input type=text name=\"keyword\"/></td><td><input type=submit name=submit value=\"キーワード追加\"/></td></tr>
    </table>
    </form> -->
    $added_keyword_message 
    <h2>ツイートの発見：</h2>
    <form action=\"webigator-demo.cgi\" method=post>
    $tweet_table
        </form>
      </div>

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
