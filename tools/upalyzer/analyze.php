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
$update_file = file('resources2.txt');
$updates = array();
$update_file_maxlen = 0;
$data_size = 0;
$data_uncompressed_size = 0;
$data_used_size = 0;

foreach ($update_file as $update_line)
{
  $splitted = split(' ', $update_line, 2);
  $file = $splitted[0];
  $update = array(
    'file' => $file,
    'adler32' => trim($splitted[1]),
    'filesize' => filesize($file));
  $update_file_maxlen = max($update_file_maxlen, strlen($file));

  $uncompressed_size = 0;
  $entries = array();
  $zip = zip_open($file);
  if ($zip) {
    while ($zip_entry = zip_read($zip)) {
      $uncompressed_size = $uncompressed_size + zip_entry_filesize($zip_entry);
      $entries[zip_entry_name($zip_entry)] = array(
        'name' => zip_entry_name($zip_entry),
        'size' => zip_entry_compressedsize($zip_entry));
    }
    zip_close($zip);
  }
  $update['entries'] = $entries;
  $update['uncompressed_size'] = $uncompressed_size;
  $updates[] = $update;

  $data_size += $update['filesize'];
  $data_uncompressed_size += $uncompressed_size;
}

function get_update_for_file($file)
{
  global $updates;

  for ($i = count($updates) - 1; $i >= 0; $i--) {
    $update = $updates[$i];
    if (array_key_exists($file, $update['entries'])) {
      return $update;
    }
  }
  return null;
}

foreach ($updates as &$update)
{
  $entry_count = count($update['entries']);
  $used_entry_count = 0;
  $used_entry_size = 0;

  foreach ($update['entries'] as $file => $entry) {
    $update_for_file = get_update_for_file($file);
    if (strcmp($update_for_file['file'], $update['file']) == 0) {
      $used_entry_count++;
      $used_entry_size += $entry['size'];
      $entry['used'] = true;
    }
  }

  $update['used_entry_count'] = $used_entry_count;
  $update['used_size'] = $used_entry_size;
  $update['used_percentage'] = $used_entry_size / $update['filesize'];

  $data_used_size += $update['used_size'];
}

foreach ($updates as &$update)
{
  echo $update['file'];
  echo str_repeat(' ', $update_file_maxlen - strlen($update['file']) + 2);
  echo $update['adler32'];
  printf("  %4d kb", $update['filesize'] / 1024);
  printf("  %4d kb", $update['uncompressed_size'] / 1024);
  printf("  %3d%% used (%d/%d files)", $update['used_percentage'] * 100,
                                       $update['used_entry_count'],
                                       count($update['entries']));
  echo "\n";
}

printf("\n");
printf("Amount of data: %4d kb\n", $data_size / 1024);
printf("Uncompressed:   %4d kb\n", $data_uncompressed_size / 1024);
printf("Obsoleted data: %4d kb (%d%%)\n",
    ($data_size - $data_used_size) / 1024,
    100 - ($data_used_size / $data_size) * 100);

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
