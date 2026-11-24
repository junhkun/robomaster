#include "app.h"

#include <stdio.h>
#include <string.h>

#include "gpio.h"
#include "OLED.h"
#include "usart.h"

#define PACKET_HEADER              0xAAU
#define PACKET_LENGTH              6U
#define DEFAULT_STEP_MS            300U
#define MIN_STEP_MS                100U
#define MAX_STEP_MS                2000U
#define KEY_DEBOUNCE_MS            20U
#define FEEDBACK_QUEUE_LENGTH      8U
#define FEEDBACK_LINE_LENGTH       48U

typedef struct __attribute__((packed))
{
  uint8_t header;
  uint8_t run;
  uint8_t direction;
  uint16_t step_ms;
  uint8_t checksum;
} ControlPacket;

typedef struct
{
  uint8_t running;
  uint8_t direction; /* 0: forward, 1: reverse */
  uint16_t step_ms;
  uint8_t current_led; /* 0..2 */
} LampState;

_Static_assert(sizeof(ControlPacket) == PACKET_LENGTH,
               "ControlPacket must contain exactly six bytes");

static LampState lamp_state;
static ControlPacket receive_packet;

static volatile uint8_t packet_ready;
static volatile uint8_t dir_key_pending;
static volatile uint8_t run_key_pending;
static volatile uint32_t dir_key_edge_tick;
static volatile uint32_t run_key_edge_tick;
static uint32_t next_switch_tick;
static uint8_t display_dirty;

static char feedback_queue[FEEDBACK_QUEUE_LENGTH][FEEDBACK_LINE_LENGTH];
static uint8_t feedback_head;
static uint8_t feedback_tail;
static uint8_t feedback_count;
static char transmit_buffer[FEEDBACK_LINE_LENGTH];
static char oled_cache[4][17];

static void start_packet_receive(void);
static void process_keys(uint32_t now);
static void process_packet(uint32_t now);
static void process_led_timer(uint32_t now);
static void process_feedback(void);
static void render_display(void);
static void render_line(char destination[17], const char *source);
static void apply_led(void);
static void queue_feedback(void);

void App_Init(void)
{
  lamp_state.running = 1;
  lamp_state.direction = 0;
  lamp_state.step_ms = DEFAULT_STEP_MS;
  lamp_state.current_led = 0;

  HAL_NVIC_SetPriority(USART1_IRQn, 2, 0);
  HAL_NVIC_EnableIRQ(USART1_IRQn);

  OLED_Init();
  apply_led();
  display_dirty = 1;
  render_display();

  next_switch_tick = HAL_GetTick() + lamp_state.step_ms;
  start_packet_receive();
  queue_feedback();
}

void App_Loop(void)
{
  uint32_t now = HAL_GetTick();

  process_keys(now);
  process_packet(now);
  process_feedback();
  process_led_timer(now);

  if (display_dirty != 0U)
  {
    render_display();
  }
}

static void start_packet_receive(void)
{
  if (HAL_UART_Receive_IT(&huart1,
                          (uint8_t *)&receive_packet,
                          (uint16_t)sizeof(receive_packet)) != HAL_OK)
  {
    /* The caller retries from the next main-loop iteration. */
  }
}

static void process_keys(uint32_t now)
{
  uint8_t changed = 0;

  if ((dir_key_pending != 0U) &&
      ((uint32_t)(now - dir_key_edge_tick) >= KEY_DEBOUNCE_MS))
  {
    dir_key_pending = 0;
    if (HAL_GPIO_ReadPin(KEY_DIR_GPIO_Port, KEY_DIR_Pin) == GPIO_PIN_RESET)
    {
      lamp_state.direction ^= 1U;
      next_switch_tick = now + lamp_state.step_ms;
      changed = 1U;
    }
  }

  if ((run_key_pending != 0U) &&
      ((uint32_t)(now - run_key_edge_tick) >= KEY_DEBOUNCE_MS))
  {
    run_key_pending = 0;
    if (HAL_GPIO_ReadPin(KEY_RUN_GPIO_Port, KEY_RUN_Pin) == GPIO_PIN_RESET)
    {
      lamp_state.running ^= 1U;
      if (lamp_state.running != 0U)
      {
        next_switch_tick = now + lamp_state.step_ms;
      }
      changed = 1U;
    }
  }

  if (changed != 0U)
  {
    display_dirty = 1;
    queue_feedback();
  }
}

static void process_packet(uint32_t now)
{
  ControlPacket packet;
  const uint8_t *bytes;
  uint8_t expected_checksum;

  if (packet_ready == 0U)
  {
    if (huart1.RxState == HAL_UART_STATE_READY)
    {
      start_packet_receive();
    }
    return;
  }

  packet_ready = 0;
  packet = receive_packet;
  bytes = (const uint8_t *)&packet;
  expected_checksum = (uint8_t)(bytes[0] + bytes[1] + bytes[2]
                              + bytes[3] + bytes[4]);

  if ((packet.header == PACKET_HEADER) &&
      (packet.run <= 1U) &&
      (packet.direction <= 1U) &&
      (packet.step_ms >= MIN_STEP_MS) &&
      (packet.step_ms <= MAX_STEP_MS) &&
      (packet.checksum == expected_checksum))
  {
    lamp_state.running = packet.run;
    lamp_state.direction = packet.direction;
    lamp_state.step_ms = packet.step_ms;
    if (lamp_state.running != 0U)
    {
      next_switch_tick = now + lamp_state.step_ms;
    }
    display_dirty = 1;
    queue_feedback();
  }

  start_packet_receive();
}

static void process_led_timer(uint32_t now)
{
  if ((lamp_state.running != 0U) &&
      ((int32_t)(now - next_switch_tick) >= 0))
  {
    if (lamp_state.direction == 0U)
    {
      lamp_state.current_led =
          (uint8_t)((lamp_state.current_led + 1U) % 3U);
    }
    else
    {
      lamp_state.current_led =
          (uint8_t)((lamp_state.current_led + 2U) % 3U);
    }

    apply_led();
    display_dirty = 1;
    next_switch_tick = now + lamp_state.step_ms;
  }
}

static void process_feedback(void)
{
  if ((huart1.gState == HAL_UART_STATE_READY) && (feedback_count > 0U))
  {
    memcpy(transmit_buffer,
           feedback_queue[feedback_tail],
           FEEDBACK_LINE_LENGTH);
    feedback_tail = (uint8_t)((feedback_tail + 1U) % FEEDBACK_QUEUE_LENGTH);
    feedback_count--;
    if (HAL_UART_Transmit_IT(&huart1,
                             (const uint8_t *)transmit_buffer,
                             (uint16_t)strlen(transmit_buffer)) == HAL_OK)
    {
      return;
    }

    feedback_tail = (uint8_t)((feedback_tail + FEEDBACK_QUEUE_LENGTH - 1U)
                              % FEEDBACK_QUEUE_LENGTH);
    feedback_count++;
  }
}

static void queue_feedback(void)
{
  char *slot;
  size_t length;

  if (feedback_count >= FEEDBACK_QUEUE_LENGTH)
  {
    return;
  }

  slot = feedback_queue[feedback_head];
  (void)snprintf(slot,
                 FEEDBACK_LINE_LENGTH,
                 "STATE=%s,DIR=%s,STEP=%u,LED=%u\r\n",
                 (lamp_state.running != 0U) ? "RUN" : "PAUSE",
                 (lamp_state.direction == 0U) ? "FWD" : "REV",
                 (unsigned int)lamp_state.step_ms,
                 (unsigned int)(lamp_state.current_led + 1U));

  length = strlen(slot);
  if (length < FEEDBACK_LINE_LENGTH)
  {
    feedback_head = (uint8_t)((feedback_head + 1U) % FEEDBACK_QUEUE_LENGTH);
    feedback_count++;
  }
}

static void apply_led(void)
{
  HAL_GPIO_WritePin(GPIOA, LED1_Pin | LED2_Pin | LED3_Pin, GPIO_PIN_RESET);

  switch (lamp_state.current_led)
  {
    case 0:
      HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);
      break;
    case 1:
      HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_SET);
      break;
    default:
      HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin, GPIO_PIN_SET);
      break;
  }
}

static void render_line(char destination[17], const char *source)
{
  uint8_t index = 0U;

  while ((index < 16U) && (source[index] != '\0'))
  {
    destination[index] = source[index];
    index++;
  }
  while (index < 16U)
  {
    destination[index] = ' ';
    index++;
  }
  destination[16] = '\0';
}

static void render_display(void)
{
  char line[17];
  char temporary[24];

  (void)snprintf(temporary,
                 sizeof(temporary),
                 "STATE: %s",
                 (lamp_state.running != 0U) ? "RUN" : "PAUSE");
  render_line(line, temporary);
  if (strcmp(line, oled_cache[0]) != 0)
  {
    OLED_ShowString(1, 1, line);
    memcpy(oled_cache[0], line, sizeof(line));
  }

  (void)snprintf(temporary,
                 sizeof(temporary),
                 "DIR: %s",
                 (lamp_state.direction == 0U) ? "FWD" : "REV");
  render_line(line, temporary);
  if (strcmp(line, oled_cache[1]) != 0)
  {
    OLED_ShowString(2, 1, line);
    memcpy(oled_cache[1], line, sizeof(line));
  }

  (void)snprintf(temporary,
                 sizeof(temporary),
                 "STEP: %ums",
                 (unsigned int)lamp_state.step_ms);
  render_line(line, temporary);
  if (strcmp(line, oled_cache[2]) != 0)
  {
    OLED_ShowString(3, 1, line);
    memcpy(oled_cache[2], line, sizeof(line));
  }

  (void)snprintf(temporary,
                 sizeof(temporary),
                 "LED: %u",
                 (unsigned int)(lamp_state.current_led + 1U));
  render_line(line, temporary);
  if (strcmp(line, oled_cache[3]) != 0)
  {
    OLED_ShowString(4, 1, line);
    memcpy(oled_cache[3], line, sizeof(line));
  }

  display_dirty = 0;
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  if (GPIO_Pin == KEY_DIR_Pin)
  {
    dir_key_edge_tick = HAL_GetTick();
    dir_key_pending = 1;
  }
  else if (GPIO_Pin == KEY_RUN_Pin)
  {
    run_key_edge_tick = HAL_GetTick();
    run_key_pending = 1;
  }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance == USART1)
  {
    packet_ready = 1;
  }
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance == USART1)
  {
    /* HAL_UART_Transmit_IT has completed and released the transmit state. */
  }
}
