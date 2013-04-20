# Language of the interface
our $UILANG = "en";

# Server location
our $SERVER = "localhost:9597";

# Directory of the demo
our $TOP_DIR = "/webigator-demo/";

# Number of candidates to show
our $TWEET_COUNT = 5;

# Whether to allow the addition of tasks through the CGI interface.
# For example, in the demo we don't want to allow addition of tasks.
our $ALLOW_TASK_ADD = 1;

# What to use for tokenization.
# Currently only "char" is supported.
our $TOKENIZE = "char";

# Whether to re-score the existing candidates after adding a keyword.
# Currently not implemented.
our $RESCORE = 0;

1;
