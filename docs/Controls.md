# Controls


## IMU Input Orientation

| Input | QC Direction     | Values   |
|-------|------------------|----------|
| Pitch | Level            | Zero     |
|       | Nose Up          | Positive |
|       | Nose Down        | Negative |
| Roll  | Level            | Zero     |
|       | Left Wing Down   | Positive |
|       | Right Wing Down  | Negative |
| Yaw   | Rotate Clockwise | Positive |
| Yaw   | Rotate CCW       | Negative |


## RC Input Orientation

| Channel   | Action   | Direction   | QC Action            |
|-----------|----------|-------------|----------------------|
| Channel 1 | Yaw      | < Left      | Rotate CCW           |
| Channel 1 | Yaw      | > Right     | Rotate CW            |
| Channel 2 | Throttle | ^ Up        | Increase Motor Speed |
| Channel 2 | Throttle | v Down      | Decrease Motor Speed |
| Channel 3 | Pitch    | ^ Forwards  | Nose Down            |
| Channel 3 | Pitch    | v Back      | Nose Up              |
| Channel 4 | Roll     | < Left      | Left Wing Down       |
| Channel 4 | Roll     | > Right     | Right Wing Down      |
| Channel 5 | EStop    | Toggle Up   | Run                  |
| Channel 5 | EStop    | Toggle Down | Stop                 |
| Channel 6 |          |             |                      |


