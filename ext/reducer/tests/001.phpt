--TEST--
group_by
--FILE--
<?php
$ret = group_by([ 
    [ 'category' => 'Food', 'amount' => 10 ],
    [ 'category' => 'Book', 'amount' => 30 ],
    [ 'category' => 'Drink', 'amount' => 3 ],
    [ 'category' => 'Drink', 'amount' => 5 ],
    [ 'category' => 'Drink', 'amount' => 8 ],
], ['category'], [
    'amount' => 'sum',
]);
print_r($ret);
--EXPECT--
