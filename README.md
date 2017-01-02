reducer
=======

map and reduce functions implemented in PHP extension.

[![Build Status](https://travis-ci.org/c9s/reducer.svg)](https://travis-ci.org/c9s/reducer)

## SYNOPSIS

```php
$rows = group_by([ 
    [ 'category' => 'Food', 'type' => 'pasta', 'amount' => 1, 'foo' => 10 ],
    [ 'category' => 'Food', 'type' => 'pasta', 'amount' => 1 ],
    [ 'category' => 'Food', 'type' => 'juice', 'amount' => 1 ],
    [ 'category' => 'Food', 'type' => 'juice', 'amount' => 1 ],
    [ 'category' => 'Book', 'type' => 'programming', 'amount' => 5 ],
    [ 'category' => 'Book', 'type' => 'programming', 'amount' => 2 ],
    [ 'category' => 'Book', 'type' => 'cooking', 'amount' => 6 ],
    [ 'category' => 'Book', 'type' => 'cooking', 'amount' => 2 ],
], ['category','type'], [
    'amount' => REDUCER_SUM,
    'cnt' => REDUCER_COUNT,
]);
print_r($ret);
```


```
Array (
    [0] => Array (
            [type] => pasta
            [amount] => 2
            [cnt] => 1
        )
    [1] => Array (
            [type] => juice
            [amount] => 2
            [cnt] => 1
        )
    [2] => Array (
            [type] => programming
            [amount] => 7
            [cnt] => 1
        )
    [3] => Array (
            [type] => cooking
            [amount] => 8
            [cnt] => 1
        )
)
```

## INSTALL

```
$ phpize
$ ./configure
$ make install
```





