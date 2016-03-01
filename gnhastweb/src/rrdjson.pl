#!@PERL@

$rrdfiledir = "/usr/pkg/share/cacti/rra/gnhast";

$res = "3600";
$start = "-24h";
$end = "now-1h";

$res = $in{'res'} if ($in{'res'} ne undef);
$start = $in{'start'} if ($in{'start'} ne undef);
$end = $in{'end'} if ($in{'end'} ne undef);

print "Content-Type: application/json\n\n";
open(FILE, "rrdtool fetch $rrdfiledir/28.FE7862040000.rrd AVERAGE -r $res -s $start -e $end|");

$first = 0;
print "data = [\n";
while (<FILE>) {
  chomp();
  ($ts, $num) = split(/: */, $_);
  $ts *= 1000;
  if ($ts != 0) {
    $newnum = sprintf("%0.3f", $num);
    print "," if ($first != 0);
    print "{time:\"$ts\",val:$newnum}\n";
    $first++;
  }
}
print "];\n";


sub ReadParse {
  local (*in) = @_ if @_;
  local ($i, $key, $val);
  # Read in text
  if ($ENV{'REQUEST_METHOD'} eq "GET") {
    $in = $ENV{'QUERY_STRING'};
  } elsif ($ENV{'REQUEST_METHOD'} eq "POST") {
    read(STDIN,$in,$ENV{'CONTENT_LENGTH'});
  }
  @in = split(/[&;]/,$in);
  foreach $i (0 .. $#in) {
    # Convert plus's to spaces
    $in[$i] =~ s/\+/ /g;
    # Split into key and value.
    ($key, $val) = split(/=/,$in[$i],2); # splits on the first =.
    # Convert %XX from hex numbers to alphanumeric
    $key =~ s/%(..)/pack("c",hex($1))/ge;
    $val =~ s/%(..)/pack("c",hex($1))/ge;
    # Associate key and value
    $in{$key} .= "\0" if (defined($in{$key})); # \0 is the multiple separator
    $in{$key} .= $val;
  }
  return scalar(@in);
}
