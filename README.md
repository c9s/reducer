reducer
=======

map and reduce functions implemented in PHP extension.

[![Build Status](https://travis-ci.org/c9s/reducer.svg)](https://travis-ci.org/c9s/reducer)
![PHP 7 ready](http://php7ready.timesplinter.ch/Codeception/Codeception/badge.svg)

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
    'total_amount' => [
        'selector' => 'amount',
        'aggregator' => REDUCER_SUM,
    ],
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

## BENCHMARKS

|Rows(N)    |PHP        |Extension   |Memroy   |
|-----------|-----------|------------|---------|
|100,000    |264ms      |98ms        |50MB     |
|1,000,000  |2,862ms    |918ms       |477MB    |

## INSTALL

```
$ phpize
$ ./configure
$ make install
```





