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
    return @{["サーバ${SERVER}への接続が失敗しました"]} if ($@);
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
my $tweet_table = "<input type=submit name=view_tweets value=\"ツイートを表示\"/>";
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
        $tweet_table .= "<tr>
                         <td><input type=radio name=\"group$i\" value=\"y\"/></td>
                         <td><input type=radio name=\"group$i\" value=\"n\"/></td>
                         <td><input type=radio name=\"group$i\" value=\"?\" checked/></td>
                         <td>$esc_text1<input type=hidden name=id$i value=\"$id\"/><input type=hidden name=text$i value=\"$esc_text2\"/></td></tr>";
    }
    $xmlrpc->{tpp}->set( utf8_flag => 0 ); # This is a hack so UTF works
    $err = "現在、読み込むツィートはありません。" if ($tweet_table eq "");
    if($err) {
        $tweet_table = "<p><font color=red>$err</font></p>";
    } else {
        $tweet_table = "<table width=800><tr><td>正</td><td>誤</td><td>??</td><td>テキスト</td></tr>$tweet_table</table>";
    }
}

##### Print the web page #####
print $cgi->header(-charset=>"utf-8");
print $cgi->start_html(-title=>"webigatorデモ");
print "
    <h1>webigatorデモ</h1>
    <p><a href='http://www.github.com/neubig/webigator'>webigator</a>を使って東日本大震災後のツイートから有用なツイートを見つけるデモです。今回のデモは「避難所や物資の情報を提供するツイート」を対象にしています。</p>
    
    <h2>ツイート一覧：</h2>
    <form action=\"index.pl\" method=post>
    $tweet_table
    </form>

    <h2>キーワード：</h2>
    <p>キーワードは検索したい情報を指す目印です。現在使用中のキーワードは: </p>
    <ul>
    ".join("\n",map{"    <li><b>$_</b><\/li>\n"} get_keywords())."
    </ul>
    <form action=\"index.pl\" method=post>
    <table>
    <tr><td colspan=2 align=center><b>キーワード追加フォーム</td></tr>
    <tr><td><input type=text name=\"keyword\"/></td><td><input type=submit name=submit value=\"キーワード追加\"/></td></tr>
    </table>
    </form>
    $added_keyword_message
";
print "<pre>".Dumper($params)."</pre>";
print $cgi->end_html;
exit;
