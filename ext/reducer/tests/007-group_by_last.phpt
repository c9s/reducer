--TEST--
group_by last
--FILE--
<?php
$rows = [ 
    [                    'category' => 'Food', 'amount' => 1, 'foo' => 10 ],
    [ 'name' => 'Mary',  'category' => 'Food', 'amount' => 1 ],
    [ 'name' => 'John',  'category' => 'Food', 'amount' => 1 ],
    [ 'name' => 'Cathy', 'category' => 'Book', 'amount' => 5 ],
    [ 'name' => 'Mary',  'category' => 'Book', 'amount' => 6 ],
    [ 'name' => 'Tesla', 'category' => 'Drink', 'amount' => 3 ],
    [ 'name' => 'Elon',  'category' => 'Drink', 'amount' => 5 ],
    [ 'name' => 'Jobs',  'category' => 'Drink', 'amount' => 8 ],
];
$ret = group_by($rows, ['category'], [
    'name' => REDUCER_AGGR_LAST,
    'amount' => REDUCER_AGGR_MAX,
]);
print_r($ret);
--EXPECT--
Array
(
    [0] => Array
        (
            [category] => Food
            [amount] => 1
            [name] => John
        )

    [1] => Array
        (
            [category] => Book
            [name] => Mary
            [amount] => 6
        )

    [2] => Array
        (
            [category] => Drink
            [name] => Jobs
            [amount] => 8
        )

)
