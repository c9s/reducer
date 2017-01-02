--TEST--
group_by min
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
    'amount' => REDUCER_MIN,
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
            [amount] => 5
        )

    [2] => Array
        (
            [category] => Drink
            [amount] => 3
        )

)
