--TEST--
group_by closure
--FILE--
<?php
$init = memory_get_usage();
$rows = [
    [ 'category' => 'Food', 'type' => 'pasta', 'amount' => 1, 'foo' => 10 ],
    [ 'category' => 'Food', 'type' => 'pasta', 'amount' => 2 ],
    [ 'category' => 'Food', 'type' => 'juice', 'amount' => 2 ],
    [ 'category' => 'Food', 'type' => 'juice', 'amount' => 3 ],
    [ 'category' => 'Book', 'type' => 'programming', 'amount' => 5 ],
    [ 'category' => 'Book', 'type' => 'programming', 'amount' => 2 ],
    [ 'category' => 'Book', 'type' => 'cooking', 'amount' => 6 ],
    [ 'category' => 'Book', 'type' => 'cooking', 'amount' => 2 ],
];
$ret = group_by($rows, ['category','type'], [
    'amount' => function($carry, $current) {
        return $carry + $current;
    }
]);
print_r($ret);
--EXPECT--
Array
(
    [0] => Array
        (
            [category] => Food
            [type] => pasta
            [amount] => 3
        )

    [1] => Array
        (
            [category] => Food
            [type] => juice
            [amount] => 5
        )

    [2] => Array
        (
            [category] => Book
            [type] => programming
            [amount] => 7
        )

    [3] => Array
        (
            [category] => Book
            [type] => cooking
            [amount] => 8
        )

)
