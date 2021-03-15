use strict;
use warnings;
use IO::Socket::INET;
use Time::HiRes qw( sleep usleep );



my @psock = (
    PeerAddr => "192.168.0.103:80",
    PeerPort => 80,     # "udp(80)",         #  "http(80)",
    Proto    => 'udp' );    # 'tcp' );
my $sock = IO::Socket::INET->new(@psock);
binmode $sock;

if (!$sock)
{
    print("ERROR could not conect to server\n");
    exit(0);
}


# my $started = 0;
# my $line = '';
# my $at = 0;

while (1)
{
    my $line;
    recv($sock,$line,7,0);
    {
        my ($x,$y,$z) = unpack("sss",$line);
        print("x=$x y=$y z=$z\n");
    }
    sleep(0.01);
    
    
    # my $c;
    # if (read($sock,$c,1)==1)    
    # {
    #     # print("c=".ord($c)."\n" );
    #      
    #     if (ord($c)==10)    # 0x0a written for \n by esp8266  # eq '\n')
    #     {
    #         if ($at == 6)
    #         {
    #             my ($x,$y,$z) = unpack("sss",$line);
    #             print("x=$x y=$y z=$z\n");
    #         }
    #         $at = 0;
    #         $line = '';
    #         $started = 1;
    #     }
    #     elsif ($started)
    #     {
    #         $line .= $c;
    #         $at++;
    #     }
    # }
    # sleep(0.01);

    # my $line = <$sock>;
    # if (defined($line))
    # {
    #     my ($x,$y,$z) = unpack("sss",$line);
    #     print("x=$x y=$y z=$z\n");
    # }
    # sleep(0.01);
}
