reducer
=======

map and reduce functions implemented in PHP extension.

[![Build Status](https://travis-ci.org/c9s/reducer.svg)](https://travis-ci.org/c9s/reducer)

## CURRENT VERSION

0.999 - API unstable

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

## AGGREGATORS

- `REDUCER_SUM`
- `REDUCER_COUNT`
- `REDUCER_MIN`
- `REDUCER_MAX`
- `REDUCER_AVG`


## INSTALL

```
$ phpize
$ ./configure
$ make install
```





