<?php
$rows = [];
$cates = ['Food', 'Book', 'Drink'];
$amounts = range(10, 200, 10);
for ($i = 0; $i < 1000000; $i++) {
    $rows[] = [
        'category' => array_rand($cates),
        'amount' => array_rand($amounts),
        'title' => 'Test Data',
    ];
}

$start = microtime(true);

$result = group_by($rows, ['category'], [
    'amount' => REDUCER_SUM,
]);

echo count($rows), " rows\n";

$duration = microtime(true) - $start;
echo $duration * 1000 , "ms\n";

$bytes = memory_get_peak_usage(false);
$megabytes = $bytes / 1024 / 1024;
echo $megabytes , "MB\n";
