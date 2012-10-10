# サーバの場所
our $SERVER = "localhost:9597";

# デモが存在するディレクトリ
our $TOP_DIR = "/shinsaidata-project/demo";

# 一回の閲覧に何ツイートを保持するか
our $TWEET_COUNT = 5;

# 単語・文字分割の方法（未実装、現在はcharのみ）
our $TOKENIZE = "char";

# キーワードやラベル付きデータを追加した後、サーバに保持しているテキストを
# 再度スコア付けするかどうか（未実装、現在は再度スコア付けをしない）
our $RESCORE = 0;

1;
