use strict;
use warnings;
use IO::Socket::INET;
use Time::HiRes qw( sleep usleep );

my $peer_addr = "192.168.0.102:80";

print "Connecting to peer at $peer_addr\n";


my @psock = (
    PeerAddr => $peer_addr,
    PeerPort => 80,         # "udp(80)",         #  "http(80)",
    Proto    => 'tcp',      # 'udp'
    Timeout  => 5,          # timeout for connection
);

my $sock = IO::Socket::INET->new(@psock);
if (!$sock)
{
    print("ERROR could not connect to server\n");
    exit(0);
}
binmode $sock;

print "Connected.\n";

while (1)
{
    my $c;
    if (read($sock,$c,1)==1)
    {
        printf("-->(0x%02x) %s\n",ord($c),(ord($c)>32?$c:' '));
    }
    sleep(0.01);
}
