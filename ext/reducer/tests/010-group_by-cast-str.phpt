--TEST--
group_by cast string to number
--FILE--
<?php
$ret = group_by([ 
    [ 'category' => 'Food', 'amount' => '100' ],
    [ 'category' => 'Food', 'amount' => '200' ],
    [ 'category' => 'Book', 'amount' => '300' ],
    [ 'category' => 'Book', 'amount' => '500' ],
], ['category'], [
    'amount' => [
        'isa'        => 'int',
        'aggregator' => REDUCER_AGGR_SUM,
    ]
]);
print_r($ret);
--EXPECT--
Array
(
    [0] => Array
        (
            [category] => Food
            [amount] => 300
        )

    [1] => Array
        (
            [category] => Book
            [amount] => 800
        )

)
