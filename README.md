reducer
=======

map and reduce functions implemented in PHP extension.

[![Build Status](https://travis-ci.org/c9s/reducer.svg)](https://travis-ci.org/c9s/reducer)
![PHP 7 ready](http://php7ready.timesplinter.ch/Codeception/Codeception/badge.svg)

## Current Status

0.999 - API unstable

## Synopsis

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

## Aggregators

Aggregator definition syntax

```php
[
    '{alias}' => [
        'selector' => '{selector}',
        'aggregator' => {constant | function},
    ],
    '{alias}' => {constant | function},
]
```

### Built-in Aggregators

- `REDUCER_SUM`
- `REDUCER_COUNT`
- `REDUCER_MIN`
- `REDUCER_MAX`
- `REDUCER_AVG`

### Aggregating data with custom user function

Aggregating with custom reduce function:


```php
$ret = group_by($rows, ['category','type'], [
    'amount' => function($carry, $current) {
        return $carry + $current;
    }
]);
```

Aggregating with selector:

```php
$result = group_by($rows, ['category','type'], [
    'total_amount' => [
        'selector'   => 'amount',
        'aggregator' => function($carry, $current) { return $carry + $current; }
    ],
]);
```

## Benchmark

|Rows(N)    |PHP        |Extension   |Memroy   |
|-----------|-----------|------------|---------|
|100,000    |264ms      |98ms        |50MB     |
|1,000,000  |2,862ms    |918ms       |477MB    |

## Install

```
$ phpize
$ ./configure
$ make install
```

## License

© 2017-01-01 Yo-an Lin ALL RIGHTS RESERVED


