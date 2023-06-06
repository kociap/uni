use HTTP::Daemon;
use HTTP::Status;
#use IO::File;

my $d = HTTP::Daemon->new(
    LocalAddr => 'localhost',
    LocalPort => 4320,
) || die;

print "Listening at ", $d->url, "\n";

while (my $c = $d->accept) {
    while (my $r = $c->get_request) {
        if ($r->method eq 'GET') {
            $c->send_file_response("./webpage/index.html");
        } else {
            $c->send_error(RC_FORBIDDEN)
        }
    }
    $c->close;
    undef($c);
}
