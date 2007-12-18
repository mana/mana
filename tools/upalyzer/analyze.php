<?php
/*  analyze.php
 *  Copyright 2007 Bjørn Lindeijer
 *
 *  This file is part of upalyzer.
 *
 *  upalyzer is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  any later version.
 *
 *  upalyzer is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with upalyzer; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
?>
<html>
<head><title>Update analysis</title></head>
<body>
<pre>
List of current updates:

<?php
$update_file = array_filter(array_reverse(file('resources2.txt')));
$updates = array();
$update_file_maxlen = 0;
$data_size = 0;
$data_uncompressed_size = 0;
$data_used_size = 0;
$data_overhead_size = 0;
$update_entries = array();
$update_entry_maxlen = 0;

foreach ($update_file as $update_line)
{
  $splitted = split(' ', $update_line, 2);
  $file = $splitted[0];
  $update = array(
    'file' => $file,
    'adler32' => trim($splitted[1]),
    'filesize' => filesize($file),
    'size' => 0,
    'used_entry_count' => 0,
    'used_size' => 0,
    'uncompressed_size' => 0);
  $update_file_maxlen = max($update_file_maxlen, strlen($file));

  $entries = array();
  $zip = zip_open($file);

  if ($zip) {
    while ($zip_entry = zip_read($zip)) {
      $update['uncompressed_size'] += zip_entry_filesize($zip_entry);
      $entry_name = zip_entry_name($zip_entry);
      $entry_size = zip_entry_compressedsize($zip_entry);
      $entry_used = !array_key_exists($entry_name, $update_entries);
      $entries[zip_entry_name($zip_entry)] = array(
        'name' => $entry_name,
        'size' => $entry_size,
        'used' => $entry_used);
      $update['size'] += $entry_size;

      if ($entry_used) {
        $update['used_entry_count']++;
        $update['used_size'] += $entry_size;
      }

      $update_entries[$entry_name] = $update['file'];
      $update_entry_maxlen = max($update_entry_maxlen, strlen($entry_name));
    }
    zip_close($zip);
  }

  $update['entries'] = $entries;
  $update['used_percentage'] = $update['used_size'] / $update['size'];
  $updates[] = $update;

  $data_used_size += $update['used_size'];
  $data_size += $update['size'];
  $data_uncompressed_size += $update['uncompressed_size'];
  $data_overhead_size += $update['filesize'] - $update['size'];
}

foreach ($updates as &$update)
{
  printf("%-{$update_file_maxlen}s  ", $update['file']);
  echo $update['adler32'];
  printf("  %4d kb", $update['filesize'] / 1024);
  printf("  %4d kb", $update['uncompressed_size'] / 1024);
  printf("  %3d%% used (%d/%d files)", $update['used_percentage'] * 100,
                                       $update['used_entry_count'],
                                       count($update['entries']));
  echo "\n";
}

printf("\n");
printf("Amount of data: %4d kb (+%d kb zip file overhead)\n",
    $data_size / 1024,
    $data_overhead_size / 1024);
printf("Uncompressed:   %4d kb\n", $data_uncompressed_size / 1024);
printf("Obsoleted data: %4d kb (%d%%)\n",
    ($data_size - $data_used_size) / 1024,
    100 - ($data_used_size / $data_size) * 100);

printf("\n");

foreach ($update_entries as $entry => $update)
{
  printf("%-{$update_entry_maxlen}s  %s\n", $entry, $update);
}

/*
if ($dh = opendir('.')) {
  while (($file = readdir($dh)) !== false) {
  }
  closedir($dh);
}
*/
?>
</pre>
</body>
</html>
