--TEST--
group_by group aggr
--FILE--
<?php
$rows = [
    [ 'user' => 'john', 'category' => 'Food', 'type' => 'pasta', 'amount' => 1, 'foo' => 10 ],
    [ 'user' => 'john', 'category' => 'Food', 'type' => 'pasta', 'amount' => 2 ],
    [ 'user' => 'john', 'category' => 'Food', 'type' => 'juice', 'amount' => 2 ],
    [ 'user' => 'john', 'category' => 'Food', 'type' => 'juice', 'amount' => 3 ],
    [ 'user' => 'john', 'category' => 'Book', 'type' => 'programming', 'amount' => 5 ],
    [ 'user' => 'elon', 'category' => 'Book', 'type' => 'programming', 'amount' => 2 ],
    [ 'user' => 'elon', 'category' => 'Book', 'type' => 'cooking', 'amount' => 6 ],
    [ 'user' => 'elon', 'category' => 'Book', 'type' => 'cooking', 'amount' => 2 ],
];
$ret = group_by($rows, ['user'], [
    'categories' => [
        'selector' => 'category',
        'aggregator' => REDUCER_AGGR_GROUP,
    ]
]);
print_r($ret);
--EXPECT--
Array
(
    [0] => Array
        (
            [user] => john
            [categories] => Array
                (
                    [0] => Food
                    [1] => Food
                    [2] => Food
                    [3] => Food
                    [4] => Book
                )

        )

    [1] => Array
        (
            [user] => elon
            [categories] => Array
                (
                    [0] => Book
                    [1] => Book
                    [2] => Book
                )

        )

)
