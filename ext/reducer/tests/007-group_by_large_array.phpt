--TEST--
group_by large array
--FILE--
<?php
$rows = [];
$cates = ['Food', 'Book', 'Drink'];
$amounts = range(10, 200, 10);
for ($i = 0; $i < 100000; $i++) {
    $rows[] = [
        'category' => array_rand($cates),
        'amount' => array_rand($amounts),
        'title' => 'Test Data',
    ];
}
$result = group_by($rows, ['category'], [
    'amount' => REDUCER_SUM,
]);

$bytes = memory_get_peak_usage(false);
$megabytes = $bytes / 1024 / 1024;
echo $megabytes < 128 ? "OK" : "FAILED";
--EXPECT--
OK
