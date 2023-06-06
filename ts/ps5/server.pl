#!/usr/bin/perl

use HTTP::Daemon;
use HTTP::Status;
use IO::File;

my $d = HTTP::Daemon->new(
    LocalAddr => 'localhost',
    LocalPort => 4322,
) || die;

print "Listening at ", $d->url, "\n";

while (my $connection = $d->accept) {
    while (my $request = $connection->get_request) {
        if (not $request->method eq 'GET') {
            $connection->send_error(RC_FORBIDDEN);
            continue;
        }

        my $url = $request->uri->path;
        if ($url eq '/') {
            $connection->send_file_response('webpage/index.html');
        } elsif ($url eq '/subpage1') {
            $connection->send_file_response('webpage/subpage1.html');
        } elsif ($url eq '/subpage2') {
            $connection->send_file_response('webpage/subpage2.html');
        } elsif ($url eq '/header') {
            # Respond with client headers.
            my $headers = $request->headers_as_string;
            my $response = HTTP::Response->new(200);
            $response->header("Content-Type" => "text/plain");
            $response->content($headers);
            $connection->send_response($response);
        } else {
            $connection->send_error(RC_NOT_FOUND)
        }
    }

    $connection->close;
    undef($connection);
}
