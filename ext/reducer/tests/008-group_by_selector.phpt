--TEST--
group_by selector
--FILE--
<?php
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
print_r($ret);
--EXPECT--
Array
(
    [0] => Array
        (
            [category] => Food
            [type] => pasta
            [total_amount] => 2
        )

    [1] => Array
        (
            [category] => Food
            [type] => juice
            [total_amount] => 2
        )

    [2] => Array
        (
            [category] => Book
            [type] => programming
            [total_amount] => 7
        )

    [3] => Array
        (
            [category] => Book
            [type] => cooking
            [total_amount] => 8
        )

)
