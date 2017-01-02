--TEST--
group_by
--FILE--
<?php
$ret = group_by([ 
    [ 'category' => 'Food', 'amount' => 1, 'foo' => 10 ],
    [ 'category' => 'Food', 'amount' => 1 ],
    [ 'category' => 'Food', 'amount' => 1 ],
    [ 'category' => 'Book', 'amount' => 5 ],
    [ 'category' => 'Book', 'amount' => 6 ],
    [ 'category' => 'Drink', 'amount' => 3 ],
    [ 'category' => 'Drink', 'amount' => 5 ],
    [ 'category' => 'Drink', 'amount' => 8 ],
], ['category'], [
    'amount' => REDUCER_AVG,
]);
print_r($ret);
--EXPECT--
Array
(
    [0] => Array
        (
            [category] => Food
            [amount] => 1
        )

    [1] => Array
        (
            [category] => Book
            [amount] => 5.5
        )

    [2] => Array
        (
            [category] => Drink
            [amount] => 5.3333333333333
        )

)
