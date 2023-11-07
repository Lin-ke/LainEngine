#pragma once

#define L_MIN(x, y) (((x) < (y)) ? (x) : (y))
#define L_MAX(x, y) (((x) > (y)) ? (x) : (y))
#define L_PIN(a, min_value, max_value) L_MIN(max_value, L_MAX(a, min_value))

#define L_VALID_INDEX(idx, range) (((idx) >= 0) && ((idx) < (range)))
#define L_PIN_INDEX(idx, range) L_PIN(idx, 0, (range)-1)

#define L_SIGN(x) ((((x) > 0.0f) ? 1.0f : 0.0f) + (((x) < 0.0f) ? -1.0f : 0.0f))
