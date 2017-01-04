--TEST--
group_by memory test
--FILE--
<?php
ini_set('memory_limit', '1024M');

$init = memory_get_usage();
$rows = [
    [ 'category' => 'Food', 'type' => 'pasta', 'amount' => 1, 'foo' => 10 ],
    [ 'category' => 'Food', 'type' => 'pasta', 'amount' => 1 ],
    [ 'category' => 'Food', 'type' => 'juice', 'amount' => 1 ],
    [ 'category' => 'Food', 'type' => 'juice', 'amount' => 1 ],
    [ 'category' => 'Book', 'type' => 'programming', 'amount' => 5 ],
    [ 'category' => 'Book', 'type' => 'programming', 'amount' => 2 ],
    [ 'category' => 'Book', 'type' => 'cooking', 'amount' => 6 ],
    [ 'category' => 'Book', 'type' => 'cooking', 'amount' => 2 ],
];
$ret = group_by($rows, ['category','type'], [
    'total_amount' => [
        'selector' => 'amount',
        'aggregator' => REDUCER_SUM,
    ]
]);

$before = memory_get_usage();

unset($ret);

$after = memory_get_usage();
echo intval($init) > intval($after) ? "OK" : "FAILED";
--EXPECT--
OK
