#!/usr/bin/perl

use strict;
use utf8;
use Getopt::Long;
use XML::RPC;
use Data::Dumper;
use List::Util qw(sum min max shuffle);
binmode STDIN, ":utf8";
binmode STDOUT, ":utf8";
binmode STDERR, ":utf8";

require "settings.pl";
our $SERVER;

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
    eval { $result = $xmlrpc->call("get_keywords"); };
    return @{["サーバ${SERVER}への接続が失敗しました"]} if ($@);
    return @{[$result->{"faultString"}]} if $result->{"faultString"};
    my @arr = map { detokenize($_) } keys %$result;
    return @arr;
}

##### Print the web page #####
print "
<html>
    <head>webigatorデモ</head>
    <body>
    <p><a href='http://www.github.com/neubig/webigator'>webigator</a>を使って東日本大震災後のツイートから有用なツイートを見つけるデモです。今回のデモは「避難所や物資の情報を提供するツイート」を対象にしています。</p>
    <h3>キーワード：</h3>
    <p>現在使用中のキーワード: <b>".join("，",get_keywords())."</b></p>
    <p>TODO: キーワード追加フォーム</p>
    <h3>ツイート：</h3>
    <p>TODO: ツイートのラベル付け</p>
    </body>
</html>";
