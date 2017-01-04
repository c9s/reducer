--TEST--
group_by immutable
--FILE--
<?php
$rows = [ 
    [ 'category' => 'Food', 'amount' => 1, 'foo' => 10 ],
    [ 'category' => 'Food', 'amount' => 1 ],
    [ 'category' => 'Food', 'amount' => 1 ],
    [ 'category' => 'Book', 'amount' => 5 ],
    [ 'category' => 'Book', 'amount' => 6 ],
    [ 'category' => 'Drink', 'amount' => 3 ],
    [ 'category' => 'Drink', 'amount' => 5 ],
    [ 'category' => 'Drink', 'amount' => 8 ],
];
$ret = group_by($rows, ['category'], [
    'amount' => REDUCER_AGGR_SUM,
    'cnt' => REDUCER_AGGR_COUNT,
]);
print_r($rows);
--EXPECT--
Array
(
    [0] => Array
        (
            [category] => Food
            [amount] => 1
            [foo] => 10
        )

    [1] => Array
        (
            [category] => Food
            [amount] => 1
        )

    [2] => Array
        (
            [category] => Food
            [amount] => 1
        )

    [3] => Array
        (
            [category] => Book
            [amount] => 5
        )

    [4] => Array
        (
            [category] => Book
            [amount] => 6
        )

    [5] => Array
        (
            [category] => Drink
            [amount] => 3
        )

    [6] => Array
        (
            [category] => Drink
            [amount] => 5
        )

    [7] => Array
        (
            [category] => Drink
            [amount] => 8
        )

)
