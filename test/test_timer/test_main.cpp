#include <unity.h>

#include <cstring>

#include "Arduino.h"
#include "Timer.h"

unsigned long fake_millis = 0;
int fake_pin_state[256] = {};
SerialMock Serial;

static int callback_count = 0;

static void count_callback() {
  callback_count++;
}

void setUp() {
  fake_millis = 0;
  callback_count = 0;
  std::memset(fake_pin_state, 0, sizeof(fake_pin_state));
}

void tearDown() {}

void test_invalid_timer_id_returns_safe_values() {
  Timer timer;
  int hour = 9;
  int minute = 9;
  int second = 9;

  TEST_ASSERT_EQUAL_UINT32(0, timer.getOperTime(-1));
  TEST_ASSERT_EQUAL_UINT32(0, timer.getOperTime(-1, &hour, &minute, &second));
  TEST_ASSERT_EQUAL_INT(0, hour);
  TEST_ASSERT_EQUAL_INT(0, minute);
  TEST_ASSERT_EQUAL_INT(0, second);
  TEST_ASSERT_EQUAL_INT(NO_TIMER_AVAILABLE, timer.getCounter(-1));
  TEST_ASSERT_EQUAL_INT(TIMER_NOT_AN_EVENT, timer.getEventType(-1));
}

void test_after_runs_once_and_stops() {
  Timer timer;

  fake_millis = 100;
  int8_t id = timer.after(1000, count_callback);
  TEST_ASSERT_GREATER_OR_EQUAL_INT(0, id);

  fake_millis = 1099;
  timer.update();
  TEST_ASSERT_EQUAL_INT(0, callback_count);

  fake_millis = 1100;
  timer.update();
  TEST_ASSERT_EQUAL_INT(1, callback_count);
  TEST_ASSERT_EQUAL_INT(1, timer.getCounter(id));
  TEST_ASSERT_EQUAL_INT(EVENT_NONE, timer.getEventType(id));
  TEST_ASSERT_EQUAL_UINT32(1000, timer.getOperTime(id));
}

void test_every_reports_elapsed_time() {
  Timer timer;

  fake_millis = 1000;
  int8_t id = timer.every(1000, count_callback);
  TEST_ASSERT_GREATER_OR_EQUAL_INT(0, id);

  fake_millis = 3661000;
  timer.update();

  int hour = 0;
  int minute = 0;
  int second = 0;
  TEST_ASSERT_EQUAL_UINT32(3660000, timer.getOperTime(id, &hour, &minute, &second));
  TEST_ASSERT_EQUAL_INT(1, hour);
  TEST_ASSERT_EQUAL_INT(1, minute);
  TEST_ASSERT_EQUAL_INT(0, second);
  TEST_ASSERT_EQUAL_INT(1, callback_count);
}

int main(int argc, char **argv) {
  UNITY_BEGIN();
  RUN_TEST(test_invalid_timer_id_returns_safe_values);
  RUN_TEST(test_after_runs_once_and_stops);
  RUN_TEST(test_every_reports_elapsed_time);
  return UNITY_END();
}
