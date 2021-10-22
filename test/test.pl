#!/usr/bin/perl

# definitions

$BGNPMPT = "-------- begin testing --------\n";
$ENDPMPT = "--------  end testing  --------\n";

$PASS = "OK\t";
$FAIL = "FAILED\t";

$DIR = "test/";
$DEMO = "demo";

$TMP_FILE = "/tmp/myftptmpvar.tmp.txt";
$CLIENT = "[MYFTP client]\n";
$SERVER = "[MYFTP server]\n";

$MAKE = "make test:\t";
$BASIC_FUNC = "basic func test:\t";

$score = 0;

$TEST = "command -v ";
$REDIR = " > /dev/null 2>&1";
$ERR = "error: ";

sub err_msg{
  print $FAIL . $_[0] . "/" . $_[1] . "\n";
  print $ERR . $_[2] . "\n";
  print "score: $score/100\n";
  if(@_ > 3){
    system($_[3]);
  }
  system("rm -rf server/ client/ $REDIR");
  exit();
}

sub pass_msg{
  print $PASS . $_[0] . "/" . $_[0] . "\n";
}

sub exec_and_get_output{
  system($_[0] . "> $TMP_FILE");
  open(my $fd, "<$TMP_FILE");
  my @var = <$fd>;
  close $fd;
  system("rm -f $TMP_FILE");
  
  my $string = "";
  foreach $_(@var){
    $string .= $_;
  }
  return $string;
}

sub sig_alarm{
  kill('TERM', @ppid);
  foreach $_(@ppid){
    waitpid($_, 0);
  }
  err_msg(0, 10, "multi thread time out.", "kill $pid $REDIR; rm -rf $TMP_FILE $REDIR");
}

sub sig_default{}

$SIG{ALRM}=\&sig_alarm;

# Testing POSIX bash commands.
# 'command' is the safest to use since its exit status is well defined.
# You should avoid 'which', which is heavily platform-dependent.
# https://stackoverflow.com/questions/592620/how-can-i-check-if-a-program-exists-from-a-bash-script#answer-677212
sub test_cmd{
  $str = $TEST . $_[0] . $REDIR;
  if(system("$str")) {
    die $ERR . $_[0] . " command not found.\n";
  }
}

test_cmd("diff");
test_cmd("make");

# begin testing
print $BGNPMPT;

# make test
print $MAKE;

## make clean
if(system("cd .. && make clean $REDIR")){
  err_msg(0, 5, "make clean failed.");
}
if(-e "../myftpclient"){
  err_msg(0, 5, "myftpclient has not been removed in make clean.");
}
if(-e "../myftpserver"){
  err_msg(0, 5, "myftpserver has not been removed in make clean.");
}
$score += 2;

## make
if(system("cd .. && make $REDIR")){
  err_msg(2, 5, "make failed.");
}
if(not -e "../myftpclient"){
  err_msg(2, 5, "myftpclient: no target file.");
}
if(not -e "../myftpserver"){
  err_msg(2, 5, "myftpserver: no target file.");
}
$score += 3;
pass_msg(5);

if(system("mkdir -p client server && cp ../myftpclient client/ " .
  "&& cp ../myftpserver server/ && cp ./demomyftpclient client/ && cp ./demomyftpserver server/")){
    err_msg(5,5, "fail to copy myftp exec files.");
}
if(system("cp access.ini server/ && cp access.txt server/ && cp JinYongsWorks.tar server/" .
" && cp binary.o server/ && cp asciitext.txt server/ && cp put.txt client/" .
" && cp recv.txt server/")){
  err_msg(5,5, "fail to copy files to source/server.");
}


# client test
print "\n" . $CLIENT;

## setup server on port 43145
$pid = exec_and_get_output("cd server && ./${DEMO}myftpserver 43145 $REDIR & echo \$! ");

## basic func test
print "basic func test:\t";

for(my $i = 1; $i <= 100; $i += 1){ 
  $string = exec_and_get_output("cd client && ./myftpclient < ../source/basic_func.txt 2> /dev/null ");

  opendir $dh, "./server" or die $!;
  @dir = grep { /^[^\.]/ } readdir $dh;
  foreach $_ (@dir) {
    if($string !~ /$_/){
      err_msg(int($i / 10), 10, "list file mismatch.", "kill $pid");
    }
  }
  $score += !($i % 10);
}
pass_msg(10);

## test asciitext
print "ascii file test:\t";

for(my $i = 1; $i <= 100; $i += 1){
  $string = exec_and_get_output("cd client && ./myftpclient < ../source/ascii_text.txt 2> /dev/null ");

  if(system("diff client/asciitext.txt server/asciitext.txt $REDIR")){
    err_msg(int($i / 10), 10, "ascii files mismatch.", "kill $pid");
  }
  $score += !($i % 10);
  system("rm -rf client/asciitext.txt $REDIR");
}
pass_msg(10);

## test binarytext
print "binary file test:\t";
for(my $i = 1; $i <= 100; $i += 1){
  $string = exec_and_get_output("cd client && ./myftpclient < ../source/binary_text.txt 2> /dev/null ");

  if(system("diff client/binary.o server/binary.o $REDIR")){
    err_msg(int($i / 10), 10, "binary files mismatch.", "kill $pid");
  }
  $score += !($i % 10);
  system("rm -rf client/binary.o $REDIR");
}
pass_msg(10);

## test large file
print "large file test:\t";
for(my $i = 1; $i <= 20; $i += 1){
  $string = exec_and_get_output("cd client && ./myftpclient < ../source/large_file.txt 2> /dev/null ");

  if(system("diff client/JinYongsWorks.tar server/JinYongsWorks.tar $REDIR")){
    err_msg(int($i / 2), 2, "binary files mismatch.", "kill $pid");
  }
  $score += !($i % 2);
  system("rm -rf client/JinYongsWorks.tar $REDIR");
}
pass_msg(10);

## put test
print "put test:\t";
for(my $i = 1; $i <= 100; $i += 1){
  $string = exec_and_get_output("cd client && ./myftpclient < ../source/put_test.txt 2> /dev/null ");

  if(system("diff client/put.txt server/put.txt $REDIR")){
    err_msg(int($i / 20), 20, "put.txt: files mismatch.", "kill $pid");
  }
  $score += !($i % 20);
  system("rm -rf server/put.txt $REDIR");
}
pass_msg(5);

## test bad auth
print "bad auth test:\t";
for(my $i = 1; $i <= 100; $i += 1){ 
  $string = exec_and_get_output("cd client && ./myftpclient < ../source/auth_reject.txt 2> /dev/null ");

  opendir $dh, "./server" or die $!;
  @dir = grep { /^[^\.]/ } readdir $dh;
  foreach $_ (@dir) {
    if($string !~ /$_/){
      err_msg(int($i / 20), 20, "list file mismatch.", "kill $pid");
    }
  }
  $score += !($i % 20);
}
pass_msg(5);

system("kill $pid $REDIR");

## delayed recv test
print "delayed recv test:\t";

$pid = exec_and_get_output("cd server && ./${DEMO}myftpserver 43145 -r $REDIR & echo \$! ");

system("rm -rf client/recv.txt $REDIR");
$string = exec_and_get_output("cd client && ./myftpclient < ../source/recv_test.txt 2> /dev/null ");

if(system("diff client/recv.txt server/recv.txt $REDIR")){
  err_msg(0, 5, "recv.txt: files mismatch.", "kill $pid");
}
$score += 5;
system("rm -rf client/recv.txt $REDIR");
pass_msg(5);

system("kill $pid $REDIR");

# server test
print "\n" . $SERVER;

## setup server on port 43145
$pid = exec_and_get_output("cd server && ./myftpserver 43145 $REDIR & echo \$! ");

## basic func test
print "basic func test:\t";
for(my $i = 1; $i <= 100; $i += 1){ 
  $string = exec_and_get_output("cd client && ./${DEMO}myftpclient < ../source/basic_func.txt 2> /dev/null ");

  opendir $dh, "./server" or die $!;
  @dir = grep { /^[^\.]/ } readdir $dh;
  foreach $_ (@dir) {
    if($string !~ /$_/){
      err_msg(int($i / 10), 10, "list file mismatch.", "kill $pid");
    }
  }
  $score += !($i % 10);
}
pass_msg(10);

## test multi thread
print "multi thread test:\t";

alarm 10;
for(my $i = 1; $i <= 10; $i += 1){
  if(!defined($ppid = fork())) {
    # fork 发生错误返回 undef
    die "Cannot create subprocess: $!";
  } elsif ($ppid == 0) {
    exec("cd client && ./${DEMO}myftpclient -t < ../source/multi_thread.txt ".
    "> $TMP_FILE.$i 2> /dev/null") || die "Exec failed: $!";
  } else {
    # 在父进程中
    push(@ppid, $ppid);
  }
}
foreach $_(@ppid){
  waitpid($_, 0);
}

$SIG{ALRM}=\&sig_default;

for(my $i = 1; $i <= 10; $i += 1){
  open(my $fd, "<$TMP_FILE.$i");
  my @var = <$fd>;
  close $fd;
  system("rm -f $TMP_FILE.$i");
  
  my $string = "";
  foreach $_(@var){
    $string .= $_;
  }

  opendir $dh, "./server" or die $!;
  @dir = grep { /^[^\.]/ } readdir $dh;
  foreach $_ (@dir) {
    if ($_ =~ /myftpserver/) {
      next;
    }
    $count = () = $string =~ /$_/g;
    if(($_ ne "binary.o" and $count != 20) or ($_ eq "binary.o" and $count < 20)){
      err_msg($i, 10, "multi thread failed.", "rm -f $TMP_FILE*; kill $pid");
    }
  }
  $score += 3;
}
pass_msg(30);

system("kill $pid $REDIR");

print $ENDPMPT;
print "\nscore: $score/100\n" . "All tests passed.\n";
system("rm -rf client/ server/ $REDIR");
