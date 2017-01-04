<?php
class Reducer {

    static public function fold($rows, $fields, array $aggregators) {
        $result = [];

        if (empty($rows)) {
            return $result;
        }

        $first = $rows[0];
        foreach ($fields as $field) {
            if (isset($first[$field])) {
                $result[$field] = $first[$field];
            }
        }

        foreach ($rows as $row) {
            foreach ($aggregators as $key => $agg) {
                $current = isset($row[$key]) ? $row[$key] : null;
                $carry = isset($result[$key]) ? $result[$key] : null;
                switch ($agg) {
                case REDUCER_AGGR_SUM:
                    if ($carry) {
                        $carry += $current;
                    } else {
                        $carry = $current;
                    }
                    $result[$key] = $carry;
                    break;
                }
            }
        }
        return $result;
    }

    static public function groupItems(array $items, $field) {
        $tmp = [];
        foreach ($items as $item) {
            $key = $item[$field];
            if (isset($tmp[$key])) {
                $tmp[$key][] = $item;
            } else {
                $tmp[$key] = [];
            }
        }
        return $tmp;
    }

    static public function groupRows($rows, $fields) {
        $groups = [$rows];
        foreach ($fields as $field) {
            $tmp = [];
            foreach ($groups as $group) {
                $g = self::groupItems($group, $field);
                foreach ($g as $n) {
                    $tmp[] = $n;
                }
            }
            $groups = $tmp;
        }
        return $groups;
    }

    static public function groupBy($rows, $fields, array $aggregators) {
        $groups = self::groupRows($rows, $fields, $aggregators);
        $result = [];
        foreach ($groups as $group) {
            $res = self::fold($group, $fields, $aggregators);
            $result[] = $res;
        }
        return $result;
    }
}


if (!extension_loaded('reducer')) {
    define('REDUCER_AGGR_SUM', 1);
    define('REDUCER_AGGR_COUNT', 2);
    function group_by($rows, $fields, $aggregators) {
        return Reducer::groupBy($rows, $fields, $aggregators);
    }
}

/*
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
    'amount' => REDUCER_AGGR_SUM,
    'cnt' => REDUCER_AGGR_COUNT,
]);
print_r($ret);
*/
