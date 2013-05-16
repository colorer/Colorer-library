
$testcount = 2;

$testcmd = "..\\colorer.exe -p$testcount ";

@list = `cat test-list.txt`;

if ($#list == -1){
  print("Empty test-list.txt\n");
  exit;
}

foreach(@list){
  chomp;
  print(" ==== Testing '$_' - $testcount times. ==== \n");

  $result = `$testcmd $_`;
  chomp($result);

  $result_list[$#result_list + 1] = $result;
  $nresult_list[$#nresult_list + 1] = $result / $testcount;

  print(" ==== Results for '$_': $result\n");

  $totalresult += $result;
}
print("\n\n ====\n ====\n Total results: $totalresult\n");



$st = scalar localtime;
$file_line  = join(', ', @list);
$time_line  = join(', ', @result_list);
$ntime_line  = join(', ', @nresult_list);


open F, ">>testResults.txt" || die "error opening";
print F <<T;

$st
 - Tested : $file_line
 - Timing : $time_line
 - nTiming: $ntime_line
 - Total  : $totalresult

T
close F;

exit;
