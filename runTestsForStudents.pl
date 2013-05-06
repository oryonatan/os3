#! /usr/local/bin/perl
use Cwd ;
use Time::HiRes qw( time );
use Statistics::Descriptive;
#use strict;

my $user = "" ;
my $timed_out = 1;
my $diff_time = 0;


if ($#ARGV >= 0) { #too much arguments
 print "usage: runTests.pl\n";
 exit;
}
run4user () ;

#if ($#ARGV == 0) { # the index of the last element of ARGV
#  $user = $ARGV[0] ;
#}
#else{



#if ($user ne "") {
#run4user ($user) ;
#}
#else {
#  print "usage: runTests.pl [user]\n";
# exit;
#}

$user="";


###############
# Subrutines: #
###############

sub run4user {
  #my ($user) = @_ ;

  
  
  `rm res.txt log.txt stderr_re.txt -rf 2>/dev/null `;
  
  `touch res.txt`;
  `touch log.txt`;
  `touch stderr_re.txt`; 

  #print "[INFO] Running tests for user: $user\n" ;

  # Create .res, .log file (overriding the previous ones).
  my $res_file = "res.txt" ;
  my $log_file = "log.txt" ;
  open LOG_FILE , "> $log_file" or die "Can't open $log_file : $!";
  open RES_FILE , "> $res_file" or die "Can't open $res_file : $!";
  open STDERR_RE , ">> stderr_re.txt" or die "Can't open stderr_re.txt : $!";


  # README info
  usersReadme ("README" , , *RES_FILE , *LOG_FILE) ;

  # Add no appeal bonus:
  print RES_FILE "no_appeal\n" ;

  #`rm *.o *.a 2>/dev/null`;
  #`rm TMP 2>/dev/null`;
  
  

  # 1. make
  #print LOG_FILE "\n------------------------------$user-------------------\n" ;
  print LOG_FILE "\n##################### Make #####################\n" ;
  #system("cp $base_dir/outputdevice.h ."); #TODO , check if this header already exist
  my $command = "make" ;
  my $status = runAndLog ($command); #runs the given command with a time barier and returns the status
  if ($status != 0 || $timed_out==1) {
    print RES_FILE "compilation(max)\n" ;

    close LOG_FILE ;
    close RES_FILE ;
    return 1 ;
  }


  # 2. copy test files and link with library
  my $tester_cc = "tester.cc" ;
  my $tester_name = "testUser" ;
  my $include = "cpp" ;
  my $grep = `grep cpp Makefile` ;#search if the word gcc exist in the file Makefile
  if (! $grep) {
    $include = "$base_dir/scripts/cpp" ;
  }
  open (STDERR, ">>$log_file");
  print LOG_FILE "[INFO] running g++ -Wall $tester_cc -o $tester_name -loutputdevice -lpthread -L. -I $include\n" ;
  $status = system ("g++ -Wall $tester_cc -o $tester_name -loutputdevice -lpthread -L. -I $include 2>> stderr_re.txt") ;

  if ($status != 0) {
    $include = "c";
    print LOG_FILE "[INFO] running g++ -Wall $tester_cc -o $tester_name -loutputdevice -lpthread -L. -I $include\n" ;
    $status = system ("g++ -Wall $tester_cc -o $tester_name -loutputdevice -lpthread -L. -I $include 2>> stderr_re.txt") ;
    if ($status != 0) {
      print LOG_FILE "[ERROR] Cannot link with library.\n" ;
      print RES_FILE "compilation (max) {Cannot link with library}\n" ;

      close LOG_FILE ;
      close RES_FILE ;

      return 1 ;
    }
  }


  # 3. clean and run tests
  cleanOuts() ;#delete out1.txt, out2.txt

  runAllTests ($tester_name , *LOG_FILE , *RES_FILE) ;


  

  # tidy up:
  #system ("make clean > /dev/null") ;
  `rm *.o *.a *.lib`;
  #unlink $tester_name ;
  cleanOuts() ;
  close LOG_FILE ;
  close RES_FILE ;
   
}


sub usersReadme {
  my ($readme ,  $res , $log) = @_ ;

  print $log "##################### README #####################\n" ;

  if (not open README , "< $readme") {
    print $log "[ERROR] Cannot open $readme\n" ;
    print $res "bad_readme(max) {cannot open README file}\n" ;
    return 0 ;
  }
  else {
    print $log "Found README file - OK\n" ;
    my $first_line = <README> ;
    chomp $first_line ;
    print $res "$first_line:\n" ;
    close README ;
    return 1 ;
  }
}

#runs the given command with a time barier and returns the status
#$command is the command to run
#$log is the file descriptor for the log file
#$need_tmp is 1 if we want to save some temporal information
#return values is ($status,$timed_out)
sub runAndLog {
  my ($command) = @_ ;
  my $timeout = 60;
  $timed_out = 1;
  my $tmp = "stderr_re.txt" ;
  #open (STDERR, ">>$tmp");
  
  my $status=2;
  eval {
    local $SIG{ALRM} = sub { die "alarm\n" }; # NB: \n required, define a local subroutine to be called on timeout
    alarm $timeout;
    #print "Command started\n";
    $status = system("$command 2>> $tmp") ;#block the father process until this finish
    #print "[DEBUG] status=$status command=$command\n";
    alarm 0;
    # If this part isn't reached, the command timed out..
    $timed_out = 0;
  };
  return $status;
}


sub cleanOuts {
  unlink ("out1.txt") ;
  unlink ("out2.txt") ;
}

#  runAllTests ($tester_name , *LOG_FILE , *RES_FILE) ;
#
# ONLY TEST9 AND TEST10 OPEN OUT2
sub runAllTests {
  my ($tester , $log , $res) = @_ ;# tester is the tester name , which is called "testUser", which is the execution file that the .cc file was compiled to 

  my ($out1,$out2) = ("out1.txt" , "out2.txt") ;

  # test 1:
  runOneTest ("1" , 0 , "init_and_close" , \&look4out1 , $tester , $log , $res , 0) ;#& is a sign for a subroutine, \& refers 
                                                                                     #to the subroutine but does not call it

  # test 2:
  runOneTest ("2" , 0 , "init_twice_same_file" , \&look4out1 , $tester , $log , $res , 0) ;

  # test 3:
  runOneTest ("3" , 0 , "one_thread_writes" , \&checkLines1out1line1 , $tester , $log , $res , 0) ;

  # test 3.5:
  print $log "\n\n$user This is actually test 3.5:\n" ;
  runOneTest ("3" , 0 , "erase_contents" , \&checkLines4out1line2 , $tester , $log , $res , 0) ;

  unlink ($out1) ;
  # test 5:
  runOneTest ("5" , 0 , "write_and_flush" , \&checkLines2out1line10 , $tester , $log , $res , 0) ;

  # test 6:
  runOneTest ("6" , "ANY" , "close_and_flush" , \&libFailure6 , $tester , $log , $res , 1) ;

  # test 7:
  runOneTest ("7" , "ANY" , "write_no_init" , \&libFailure7 , $tester , $log , $res , 1) ;

  # test 8:
  runOneTest ("8" , "ANY" , "illegal_flush" , \&libFailure8 , $tester , $log , $res , 1) ;

  # test 9:
  unlink ($out2) ;
  runOneTest ("9" , 0 , "thread_write" , \&checkLines3out2line300 , $tester , $log , $res , 0) ;

  # test 10:
  unlink ($out2) ;
  runOneTest ("10" , 0 , "big_buffers" , \&checkLines3out2line300 , $tester , $log , $res , 0) ;

  # test 11:
  runOneTest ("11" , "ANY" , "close_and_write" , \&libFailure11 , $tester , $log , $res , 1) ;
  
  cleanOuts();

  # test 12:
  runOneTest ("12", 0, "run_many_threads_flush", \&checkLines5out1line300test12, $tester, $log, $res, 1);

  cleanOuts();
  # test 13:
  runOneTest ("13", 0, "run_many_threads", \&checkLines6out1line300test13, $tester, $log, $res, 1);


# test 14:
  runOneTest ("14", "ANY", "illegal_was_it_written", \&libFailure14, $tester, $log, $res, 1);

# test 15:
  runOneTest ("15", 0, "was_it_written", \&legalWasItWrriten, $tester, $log, $res, 1);

# test 16:
  unlink ($out2) ;
  runOneTest ("16", "ANY", "blocking_test", \&checkBlocking16 , $tester , $log , $res , 0) ;

}

#expected is the expected status of the output of the test, it may be "ANY" means it does not meter
#runOneTest ("1" , 0 , "init_and_close" , \&look4out1 , $tester , $log , $res , 0) ;#& is a sign for a subroutine, \& refers the routine without calling  

#$tester is the name of the executable file that runs the tests,
#$log is the log file
#$res is the results file (the for the error codes 
#runOneTest ("11" , "ANY" , "close_and_write" , \&libFailure , $tester , $log , $res , 1) ;
sub runOneTest {
  my ($testNum , $expected , $errcode , $checkFunc , $tester , $log , $res , $need_tmp) = @_ ;

  #print $log "\nTEST $testNum\n" ;
  my $start_time = time();
  my $status = runAndLog ("$tester $testNum") ;
  my $end_time = time();
  #print "[DEBUG] status=$status test num=$testNum\n";
  $diff_time = $end_time - $start_time;
  #$status = $status % 256 ; #TODO, what is this for?
  print $log "$user test $testNum ";

  if($timed_out eq 1){
    print $log " - FAILED - Timed out\n" ;
    print $res "$errcode (max) {Operation timed out - running your driver took too long}\n" ;
    $timed_out=0;
    return;
  }

  #if the $status is wrong
  if ($expected ne "ANY" and $status != $expected) {

    print $log "$user Wrong return status: expected = $expected , returned = $status\n" ;
    print $log " - FAILED" ;
    print $res "$errcode (max)\n" ;
  } 
  else {
    my $check = &$checkFunc ($log);
    if($check==1){
      print $log " - PASSED" ;
    } 
    if($check==-1){
      print $log " - FAILED WRONG STATUS" ;
      print $res "$errcode"."_status (max)\n" ;
    }
    if($check==0){
      print $log " - FAILED" ;
      print $res "$errcode (max)\n" ;
    }
  }

  print $log "  took\t $diff_time\n";
}

sub look4out1 {
  my ($log) = @_ ;
  return (look4file ("out1.txt" , $log)) ;
}

sub look4out2 {
  my ($log) = @_ ;
  return (look4file ("out2.txt" , $log)) ;
}

sub look4out12 {
  my ($log) = @_ ;
  return (look4out1($log) and look4out2($log)) ;
}

#returns 1 if the file was found
sub look4file {
  my ($file , $log) = @_ ;

  if (not -e $file) {
    print $log "$user Cannot find file $file.\n" ;
    return 0 ;
  } else {
    #print $log "$user Found file $file - OK\n" ;
    return 1 ;
  }
}


sub checkLines1out1line1 {
  my ($log) = @_ ;
  return checkLines ("out1.txt" , 1 , $log) ;
}


sub checkLines2out1line10 {
  my ($log) = @_ ;
  return checkLines ("out1.txt" , 10 , $log) ;
}

sub checkLines3out2line300 {
  my ($log) = @_ ;
  return checkLines ("out2.txt" , 300 , $log) ;
}

sub checkLines4out1line2 {
  my ($log) = @_ ;
  return checkLines ("out1.txt" , 2 , $log) ;
}

sub checkLines5out1line300test12 {
  my ($log) = @_ ;
  return checkLinesTimes("out1.txt", 300, $log, "test 12");
}

sub checkLines6out1line300test13 {
  my ($log) = @_ ;
  return checkLinesTimes("out1.txt", 300, $log, "test 13");
}

#return one if succes  - the number of lines is what expected and each line is in th eright format
sub checkLines {
  my ($out , $expected_lines , $log) = @_ ;

  if (not look4file ($out , $log)) {
    return 0 ;
  }

  # Sort out file:
  my $tmp = "TMP_LINES" ;
  system ("sort $out > $tmp") ;

  open OUT , "< $tmp" or die "[ERROR] Can't open $tmp\n" ;
  my $line_num = 0 ;
  while (defined ($line = <OUT>)) {
    next if ($line =~ m/^(\s)*$/) ;
    $line_num++ ;
    if ($line !~ m/^Thread (\d+) \((\d+)\): ((writer\d(\d?) )+)(((_BIG_){1000})?)$/) {
      print $log "$user Pattern mismatch in line $line_num: $line\n" ;
      unlink $tmp;
      return 0 ;
    }
  }

  unlink $tmp ;

  
  if ($line_num != $expected_lines) {
    print $log "$user Wrong number of lines in file. Expected = $expected_lines , Actual = $line_num\n" ;
    return 0 ;
  }

  return 1 ;
}



#sub checkLines1out1line1 {
#  my ($log) = @_ ;
#  return checkLines ("out1.txt" , 1 , $log) ;
#}

sub checkBlocking16{
  my ($log) = @_;
  checkBlocking("out2.txt",1500,$log);
}
#check that the number of printed lines is smaller then what was acctually printed
sub checkBlocking {
  my ($out ,$expected_lines_smaller_than , $log) = @_ ;

  if (not look4file ($out , $log)) {
    return 0 ;
  }
  # Sort out file:
  my $tmp = "TMP_LINES" ;
  system ("sort $out > $tmp") ;

  open OUT , "< $tmp" or die "[ERROR] Can't open $tmp\n" ;
  my $line_num = 0 ;
  while (defined ($line = <OUT>)) {
    next if ($line =~ m/^(\s)*$/) ;
    $line_num++ ;
    #if ($line !~ m/^Thread (\d+) \((\d+)\): ((writer\d(\d?) )+)(((_BIG_){1000})?)$/) {
    #  print $log "$user Pattern mismatch in line $line_num: $line\n" ;
    #  unlink $tmp;
    #  return 0 ;
    #}
  }

  unlink $tmp ;

  #print "num of lines: $line_num\n";
  if ($line_num >= $expected_lines_smaller_than) {
    print $log "$user everything was printed meaning the implementation was blocking" ;
    return 0 ;
  }

  return 1 ;
}

#very similar to the other function
#return checkLinesTimes("out1.txt", 300, $log, "test 12");
sub checkLinesTimes {
  my ($out , $expected_lines , $log, $test_name) = @_ ;
  
  if (not look4file ($out , $log)) {
    return 0 ;
  }
  
  # Sort out file:
  my $tmp = "TMP_LINES" ;
  system ("sort -n $out > $tmp") ;
  
  my $stat_write_times = Statistics::Descriptive::Full->new();
  my $stat_flush_times = Statistics::Descriptive::Full->new();

  open OUT , "< $tmp" or die "[ERROR] Can't open $tmp\n" ;
  my $line_num = 0 ;
  while (defined ($line = <OUT>)) {
    next if ($line =~ m/^(\s)*$/) ;
    $line_num++ ;
    if (    $line =~ m/^Thread (\d+)\t(before)\t(\d+)$/ ) {
      # print $log "$user before $3\n"; 
    }
    elsif ( $line =~ m/^Thread (\d+)\t(write)\t(\d+)$/ ) {
      # print $log "$user write $3\n";
      $stat_write_times->add_data($3);
    }
    elsif ( $line =~ m/^Thread (\d+)\t(flush)\t(\d+)$/ ) {
      # print $log "$user flush $3\n";
      $stat_flush_times->add_data($3);
    }
    else {
      print $log "$user Pattern mismatch in line $line_num: $line\n" ;
      unlink $tmp;
      return 0 ;
    }
  }
  unlink $tmp ;

  if ($line_num != $expected_lines) {
    print $log "$user Wrong number of lines in file. Expected = $expected_lines , Actual = $line_num\n" ;
    return 0 ;
  }

  my $mean = $stat_write_times->mean();
  my $median = $stat_write_times->median();
  #print $log "$user $test_name write $mean $median ".$stat_write_times->sample_range()."\n";

  $mean = $stat_flush_times->mean();
  $median = $stat_flush_times->median();
  #print $log "$user $test_name flush $mean $median ".$stat_flush_times->sample_range()."\n";

  return 1 ;
}


sub libFailure6 {
  my ($log) = @_;
  return libFailure($log,6,0,0);
}

sub libFailure7 {
  my ($log) = @_;
  return libFailure($log,7,1,-1);
}
sub libFailure8 {
  my ($log) = @_;
  return libFailure($log,8,0,0);
}
sub libFailure11 {
  my ($log) = @_;
  return libFailure($log,11,1,-1);
}

sub libFailure14{
  my ($log) = @_;
  return libFailure($log,14,1,-2);
}


# test 14:
#  runOneTest ("14", 0, "illegal_was_it_written", \&libFailure14, $tester, $log, $res, 1);

#call this function if you caused a device library error on pupuse and you want to check if an error was indeed ocurred
#$search_status=1 if we also have to look for negative status
#$expected - the expected status to search for in case $search_status=1

#return value:
#1 - the tester passed
#0 - the tester failed, (no error was written or error was written but the status was not what expected)
#-1 - the tester failed in the status (error message was written
sub libFailure {
  my ($log,$test_num,$search_status,$expected) = @_ ;
  open STDERR_RE , "< stderr_re.txt" or die "[ERROR] Cannot open stderr_re.txt\n" ;
  
  my $regex1 = "$test_num Output device library error";
  my $regex2 = "$test_num system error";
  while (defined ($line = <STDERR_RE>)) {
    chomp $line ;
    if($line =~ m/($regex1)/ || $line =~ m/($regex2)/) {
      
      #that is the case that we not only look for error message but also negative number status
      #the status should be the next line 
      if($search_status==1){
        while (defined ($line = <STDERR_RE>)) {
          chomp $line;
          if($line =~ m/^STATUS=$expected/){
            close STDERR_RE;
            return 1;
          }
        }
        close STDERR_RE;
        return -1;
      }
      else{
        close STDERR_RE;
        return 1
      }
    }
  }
    return 0;
}

# test 15:
#  runOneTest ("15", 0, "was_it_written", \&legalWasItWrriten, $tester, $log, $res, 1);
sub legalWasItWrriten{
  my ($log) = @_ ;
  open STDERR_RE , "< stderr_re.txt" or die "[ERROR] Cannot open stderr_re.txt\n" ;
  
  
  while (defined ($line = <STDERR_RE>)) {
    chomp $line ;
    if($line =~ m/15 STATUS=[10]/) {
      close STDERR_RE;
      return 1;
    }
  }
  return 0;
}
