# FydeOS accelerometer setup flags:

## fydeos-accel-config
source code:

```
const char kFydeOSConfig[6][3][2] ={
        {"x", "y", "z"},
        {"y", "x", "z"},
        {"z", "y", "x"},
        {"z", "x", "y"},
        {"x", "z", "y"},
        {"y", "z", "x"},
    };
```

type: int
range: 0~5
purpose: setting the axises mapping between kernel driver and chrome, enable fydeos accelerometer driver.
example: "--fydeos-accel-config=0"

## fydeos-accel-pattern
source code:

```
struct DataPattern {
  int data_size;
  int data_index[3];
  };

const struct DataPattern KFydeOSDataPattern[2] = {
    {6, {0, 1, 2}},
    {12, {0, 1, 3}},
  };
```
type: int
range: 0~1
purpose: 0: system original pattern.
         1: hid_accel_3d pattern, support surface pro3, etc.
example: "--fydeos-accel-pattern=1"

## fydeos-accel-revert-x
type: bool
purpose: revert X axis in chrome

## fydeos-accel-revert-y
as above

## fydeos-accel-revert-z
as above

## fydeos-accel-right-move
type: int
range: 4~12
purpose: some acceleromer driver use less 16-bit integer as data, if it use bit 16 to bit 4 represent 12bits integer, we should right move 4 bits to get the right data.
example: "--fydeos-accel-right-move=4"
