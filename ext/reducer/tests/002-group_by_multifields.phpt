--TEST--
group_by multiple fields
--FILE--
<?php
$ret = group_by([ 
    [ 'category' => 'Food', 'type' => 'pasta', 'amount' => 1, 'foo' => 10 ],
    [ 'category' => 'Food', 'type' => 'pasta', 'amount' => 1 ],
    [ 'category' => 'Food', 'type' => 'juice', 'amount' => 1 ],
    [ 'category' => 'Food', 'type' => 'juice', 'amount' => 1 ],
    [ 'category' => 'Book', 'type' => 'programming', 'amount' => 5 ],
    [ 'category' => 'Book', 'type' => 'programming', 'amount' => 2 ],
    [ 'category' => 'Book', 'type' => 'cooking', 'amount' => 6 ],
    [ 'category' => 'Book', 'type' => 'cooking', 'amount' => 2 ],
], ['category','type'], [
    'amount' => REDUCER_AGGR_SUM,
]);
print_r($ret);
--EXPECT--
Array
(
    [0] => Array
        (
            [category] => Food
            [type] => pasta
            [amount] => 2
        )

    [1] => Array
        (
            [category] => Food
            [type] => juice
            [amount] => 2
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
