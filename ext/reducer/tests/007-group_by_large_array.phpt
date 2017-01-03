--TEST--
group_by large array
--FILE--
<?php
$rows = [];
$cates = ['Food', 'Book', 'Drink'];
$amounts = range(10, 200, 10);
for ($i = 0; $i < 50000; $i++) {
    $rows[] = [
        'category' => array_rand($cates),
        'amount' => array_rand($amounts),
    ];
}
group_by($rows, ['category'], [
    'amount' => REDUCER_SUM,
]);
--EXPECT--
