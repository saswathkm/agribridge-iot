#include <ctype.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hardware/adc.h"
#include "hardware/gpio.h"
#include "hardware/spi.h"
#include "pico/stdlib.h"
#include "pico/time.h"

// ================== RADIO CONFIGURATION ==================
#define SPI_PORT spi0
#define SPI_BAUD 1000000
#define PIN_MISO 16
#define PIN_CS 17
#define PIN_SCK 18
#define PIN_MOSI 19
#define PIN_RESET 20
#define PIN_DIO0 21
#define LED_PIN PICO_DEFAULT_LED_PIN

// Both LoRa nodes must use matching radio settings and sync word.
static uint32_t frequency_hz = 433000000UL;
static uint8_t spreading_factor = 7;  // 7..12
static uint32_t bandwidth_hz = 125000;
static uint8_t coding_rate = 5;       // 4/5 .. 4/8
static uint8_t tx_power_dbm = 13;     // 2..17 dBm

// ================== SENSOR / RELAY CONFIGURATION ==================
// RP2040 ADC GPIOs: GP26/ADC0 voltage, GP27/ADC1 current, GP28/ADC2 soil.
#define VOLTAGE_ADC_INPUT 0
#define CURRENT_ADC_INPUT 1
#define SOIL_ADC_INPUT 2
#define DHT11_DATA_PIN 4

// Relay outputs are active-high by default. Set to 0 for active-low relay boards.
#define RELAY_1_PIN 2
#define RELAY_2_PIN 3
#define RELAY_ACTIVE_LEVEL 1

// Tune these values to the actual sensor variants and calibrated modules.
#define TELEMETRY_INTERVAL_MS_DEFAULT 5000u
#define SENSOR_SAMPLE_INTERVAL_MS 1000u
#define DHT11_SAMPLE_INTERVAL_MS 2000u
#define DHT11_TIMEOUT_US 100u
#define DHT11_ONE_THRESHOLD_US 40u
#define ADC_RMS_SAMPLE_COUNT 400u
#define ADC_SAMPLE_PERIOD_US 500u
#define VOLTAGE_CALIBRATION_V_PER_ADC_V 1000.0f
#define ACS712_SENSITIVITY_V_PER_A 0.066f  // 30 A ACS712; 0.185 for 5 A, 0.100 for 20 A
#define CURRENT_ADC_DIVIDER_RATIO 1.5f  // 1 kOhm series, 2 kOhm to ground.
#define CURRENT_ZERO_NOISE_MULTIPLIER 2.0f
#define CURRENT_CALIBRATION_POINT_COUNT 8u
#define SOIL_DRY_ADC_RAW 3000u
#define SOIL_WET_ADC_RAW 1200u
#define SOIL_LOW_THRESHOLD_PERCENT 30.0f
#define SOIL_HIGH_THRESHOLD_PERCENT 80.0f
#define LOW_VOLTAGE_THRESHOLD_V 180.0f
#define OVERCURRENT_THRESHOLD_A_DEFAULT 10.0f

#define MAX_PAYLOAD_LENGTH 255u
#define MAX_COMMAND_LENGTH 255u
#define ALERT_BUFFER_LENGTH 128u
#define RADIO_TX_TIMEOUT_US 15000000u

// SX1276/SX1278 register addresses.
#define REG_FIFO 0x00
#define REG_OP_MODE 0x01
#define REG_FR_MSB 0x06
#define REG_FR_MID 0x07
#define REG_FR_LSB 0x08
#define REG_PA_CONFIG 0x09
#define REG_LNA 0x0C
#define REG_FIFO_ADDR_PTR 0x0D
#define REG_FIFO_TX_BASE_ADDR 0x0E
#define REG_FIFO_RX_BASE_ADDR 0x0F
#define REG_IRQ_FLAGS 0x12
#define REG_RX_NB_BYTES 0x13
#define REG_PAYLOAD_LENGTH 0x22
#define REG_MODEM_CONFIG_1 0x1D
#define REG_MODEM_CONFIG_2 0x1E
#define REG_PREAMBLE_MSB 0x20
#define REG_PREAMBLE_LSB 0x21
#define REG_FIFO_RX_CURRENT_ADDR 0x10
#define REG_MODEM_CONFIG_3 0x26
#define REG_SYNC_WORD 0x39
#define REG_DIO_MAPPING_1 0x40
#define REG_VERSION 0x42
#define REG_PA_DAC 0x4D
#define REG_PKT_RSSI_VALUE 0x1A
#define REG_PKT_SNR_VALUE 0x19

#define MODE_LONG_RANGE_MODE 0x80
#define MODE_SLEEP 0x00
#define MODE_STDBY 0x01
#define MODE_TX 0x03
#define MODE_RX_CONTINUOUS 0x05

#define IRQ_TX_DONE_MASK 0x08
#define IRQ_RX_DONE_MASK 0x40
#define IRQ_PAYLOAD_CRC_ERROR_MASK 0x20

typedef struct {
    float voltage_v;
    float current_a;
    float soil_percent;
    float temperature_c;
    float humidity_percent;
    bool dht11_valid;
} sensor_readings_t;

typedef enum {
    RADIO_TX_PENDING,
    RADIO_TX_SUCCEEDED,
    RADIO_TX_FAILED
} radio_tx_status_t;

typedef enum {
    SENSOR_SAMPLING_IDLE,
    SENSOR_SAMPLING_VOLTAGE,
    SENSOR_SAMPLING_CURRENT
} sensor_sampling_phase_t;

typedef struct {
    sensor_sampling_phase_t phase;
    unsigned int sample_count;
    double mean;
    double square_sum;
    uint64_t next_sample_us;
    float voltage_v;
    float voltage_sensor_rms_v;
    float current_a;
} sensor_sampler_t;

static uint32_t telemetry_interval_ms = TELEMETRY_INTERVAL_MS_DEFAULT;
static float voltage_calibration_v_per_adc_v =
    VOLTAGE_CALIBRATION_V_PER_ADC_V;
static float overcurrent_threshold_a = OVERCURRENT_THRESHOLD_A_DEFAULT;
static float low_voltage_threshold_v = LOW_VOLTAGE_THRESHOLD_V;
static float soil_low_threshold_percent = SOIL_LOW_THRESHOLD_PERCENT;
static float soil_high_threshold_percent = SOIL_HIGH_THRESHOLD_PERCENT;
static float current_zero_current_raw;
static float current_zero_noise_rms_raw;
static float current_sensitivity_v_per_a = ACS712_SENSITIVITY_V_PER_A;
static struct {
    float sensor_rms_voltage_v;
    float reference_current_a;
} current_calibration_points[CURRENT_CALIBRATION_POINT_COUNT];
static size_t current_calibration_point_count;
static bool relay_1_on;
static bool relay_2_on;
static bool overcurrent_active;
static bool low_voltage_active;
static bool soil_low_active;
static bool soil_high_active;
static bool sensor_readings_valid;
static bool radio_tx_active;
static bool radio_configuration_pending;
static uint64_t radio_tx_deadline_us;
static sensor_sampler_t sensor_sampler;
static sensor_readings_t readings;
static char node_name[24] = "RP2040";
static char pending_alerts[ALERT_BUFFER_LENGTH] = "";
static char transmitting_alerts[ALERT_BUFFER_LENGTH] = "";

static void ra02_cs_select(void) { gpio_put(PIN_CS, 0); }
static void ra02_cs_deselect(void) { gpio_put(PIN_CS, 1); }

static uint8_t ra02_read_reg(uint8_t reg) {
    uint8_t address = reg & 0x7F;
    uint8_t value = 0;
    ra02_cs_select();
    spi_write_blocking(SPI_PORT, &address, 1);
    spi_read_blocking(SPI_PORT, 0, &value, 1);
    ra02_cs_deselect();
    return value;
}

static void ra02_write_reg(uint8_t reg, uint8_t value) {
    uint8_t data[2] = {(uint8_t)(reg | 0x80), value};
    ra02_cs_select();
    spi_write_blocking(SPI_PORT, data, 2);
    ra02_cs_deselect();
}

static void ra02_reset(void) {
    gpio_put(PIN_RESET, 0);
    sleep_ms(10);
    gpio_put(PIN_RESET, 1);
    sleep_ms(10);
}

static void ra02_set_frequency(uint32_t freq_hz) {
    uint64_t frf = ((uint64_t)freq_hz << 19) / 32000000ULL;
    ra02_write_reg(REG_FR_MSB, (uint8_t)(frf >> 16));
    ra02_write_reg(REG_FR_MID, (uint8_t)(frf >> 8));
    ra02_write_reg(REG_FR_LSB, (uint8_t)frf);
}

static void ra02_configure_lora(void) {
    ra02_write_reg(REG_OP_MODE, MODE_SLEEP);
    sleep_ms(10);
    ra02_write_reg(REG_OP_MODE, MODE_LONG_RANGE_MODE | MODE_SLEEP);
    sleep_ms(10);

    uint8_t bandwidth_bits;
    switch (bandwidth_hz) {
        case 125000: bandwidth_bits = 0x70; break;
        case 250000: bandwidth_bits = 0x80; break;
        case 500000: bandwidth_bits = 0x90; break;
        default: bandwidth_bits = 0x70; break;
    }
    uint8_t coding_rate_bits = (uint8_t)((coding_rate - 4u) << 1);
    ra02_write_reg(REG_MODEM_CONFIG_1, (uint8_t)(bandwidth_bits | coding_rate_bits));
    ra02_write_reg(REG_MODEM_CONFIG_2, (uint8_t)((spreading_factor << 4) | 0x04));
    uint8_t modem_config_3 = 0x04; // Automatic gain control.
    if (((uint32_t)1u << spreading_factor) * 1000u >=
        16u * bandwidth_hz) {
        modem_config_3 |= 0x08; // Low data rate optimization.
    }
    ra02_write_reg(REG_MODEM_CONFIG_3, modem_config_3);
    ra02_write_reg(REG_SYNC_WORD, 0x12);

    // PA_BOOST output power = tx_power_dbm; keep the +20 dBm mode disabled.
    ra02_write_reg(REG_PA_DAC, 0x84);
    ra02_write_reg(REG_PA_CONFIG, (uint8_t)(0x80 | (tx_power_dbm - 2u)));
    ra02_write_reg(REG_LNA, 0x23);
    ra02_write_reg(REG_FIFO_TX_BASE_ADDR, 0x00);
    ra02_write_reg(REG_FIFO_RX_BASE_ADDR, 0x00);
    ra02_write_reg(REG_DIO_MAPPING_1, 0x00);
    ra02_write_reg(REG_PREAMBLE_MSB, 0x00);
    ra02_write_reg(REG_PREAMBLE_LSB, 12);
    ra02_set_frequency(frequency_hz);
    ra02_write_reg(REG_OP_MODE, MODE_LONG_RANGE_MODE | MODE_STDBY);
    sleep_ms(10);
}

static bool ra02_begin_packet_transmit(const uint8_t *data, size_t length) {
    if (length == 0 || length > MAX_PAYLOAD_LENGTH) {
        return false;
    }

    ra02_write_reg(REG_OP_MODE, MODE_LONG_RANGE_MODE | MODE_STDBY);
    ra02_write_reg(REG_IRQ_FLAGS, 0xFF);
    ra02_write_reg(REG_FIFO_ADDR_PTR, 0x00);
    uint8_t fifo_address = REG_FIFO | 0x80;
    ra02_cs_select();
    spi_write_blocking(SPI_PORT, &fifo_address, 1);
    size_t bytes_written = spi_write_blocking(SPI_PORT, data, length);
    ra02_cs_deselect();
    if (bytes_written != length) {
        ra02_write_reg(REG_OP_MODE, MODE_LONG_RANGE_MODE | MODE_STDBY);
        ra02_write_reg(REG_IRQ_FLAGS, 0xFF);
        return false;
    }
    ra02_write_reg(REG_PAYLOAD_LENGTH, (uint8_t)length);
    ra02_write_reg(REG_OP_MODE, MODE_LONG_RANGE_MODE | MODE_TX);
    radio_tx_deadline_us = time_us_64() + RADIO_TX_TIMEOUT_US;
    return true;
}

static radio_tx_status_t ra02_poll_packet_transmit(void) {
    if (ra02_read_reg(REG_IRQ_FLAGS) & IRQ_TX_DONE_MASK) {
        ra02_write_reg(REG_IRQ_FLAGS, 0xFF);
        ra02_write_reg(REG_OP_MODE, MODE_LONG_RANGE_MODE | MODE_STDBY);
        return RADIO_TX_SUCCEEDED;
    }
    if (time_us_64() >= radio_tx_deadline_us) {
        ra02_write_reg(REG_OP_MODE, MODE_LONG_RANGE_MODE | MODE_STDBY);
        ra02_write_reg(REG_IRQ_FLAGS, 0xFF);
        return RADIO_TX_FAILED;
    }
    return RADIO_TX_PENDING;
}

static void debug_print_packet(const char *direction, const uint8_t *data,
                               size_t length) {
    printf("[LoRa %s] length=%u ASCII=\"", direction, (unsigned int)length);
    for (size_t i = 0; i < length; ++i) {
        unsigned char byte = data[i];
        if (byte >= 0x20 && byte <= 0x7E && byte != '\\' && byte != '"') {
            putchar(byte);
        } else if (byte == '\\' || byte == '"') {
            putchar('\\');
            putchar(byte);
        } else {
            printf("\\x%02X", byte);
        }
    }

    printf("\"\n[LoRa %s] HEX=", direction);
    for (size_t i = 0; i < length; ++i) {
        printf("%s%02X", i == 0 ? "" : " ", data[i]);
    }
    putchar('\n');
}

static void ra02_start_rx(void) {
    ra02_write_reg(REG_IRQ_FLAGS, 0xFF);
    ra02_write_reg(REG_OP_MODE, MODE_LONG_RANGE_MODE | MODE_RX_CONTINUOUS);
}

static void ra02_apply_configuration(void) {
    if (radio_tx_active) {
        radio_configuration_pending = true;
        return;
    }
    ra02_configure_lora();
    ra02_start_rx();
}

static bool ra02_receive_packet(char *buffer, size_t buffer_size,
                                size_t *received_length) {
    *received_length = 0;
    uint8_t irq_flags = ra02_read_reg(REG_IRQ_FLAGS);
    if ((irq_flags & IRQ_RX_DONE_MASK) == 0) {
        return false;
    }

    ra02_write_reg(REG_OP_MODE, MODE_LONG_RANGE_MODE | MODE_STDBY);
    if (irq_flags & IRQ_PAYLOAD_CRC_ERROR_MASK) {
        ra02_write_reg(REG_IRQ_FLAGS, 0xFF);
        printf("LoRa packet rejected: CRC error\n");
        ra02_start_rx();
        return false;
    }

    uint8_t length = ra02_read_reg(REG_RX_NB_BYTES);
    uint8_t fifo_address = ra02_read_reg(REG_FIFO_RX_CURRENT_ADDR);
    ra02_write_reg(REG_FIFO_ADDR_PTR, fifo_address);
    size_t copy_length = length;
    if (copy_length >= buffer_size) {
        copy_length = buffer_size - 1;
    }
    for (size_t i = 0; i < copy_length; ++i) {
        buffer[i] = (char)ra02_read_reg(REG_FIFO);
    }
    buffer[copy_length] = '\0';
    *received_length = copy_length;
    ra02_write_reg(REG_IRQ_FLAGS, 0xFF);
    return true;
}

static void append_alert(const char *alert) {
    size_t used = strlen(pending_alerts);
    if (used >= sizeof(pending_alerts) - 1) {
        return;
    }
    (void)snprintf(pending_alerts + used, sizeof(pending_alerts) - used,
                   "%s%s", used == 0 ? "" : "|", alert);
}

static void set_relay(unsigned int relay_number, bool on, const char *source) {
    bool *state = relay_number == 1 ? &relay_1_on : &relay_2_on;
    uint pin = relay_number == 1 ? RELAY_1_PIN : RELAY_2_PIN;
    if (*state == on) {
        return;
    }
    *state = on;
    gpio_put(pin, on ? RELAY_ACTIVE_LEVEL : !RELAY_ACTIVE_LEVEL);
    char alert[40];
    (void)snprintf(alert, sizeof(alert), "RELAY%u_%s", relay_number, on ? "ON" : "OFF");
    append_alert(alert);
    printf("Relay %u %s (%s)\n", relay_number, on ? "ON" : "OFF", source);
}

static float adc_raw_to_volts(float raw) {
    return raw * (3.3f / 4095.0f);
}

static void wait_for_next_adc_sample(absolute_time_t *next_sample) {
    *next_sample = delayed_by_us(*next_sample, ADC_SAMPLE_PERIOD_US);
    busy_wait_until(*next_sample);
}

static bool dht11_wait_for_level(bool high) {
    uint32_t start_us = time_us_32();
    while ((gpio_get(DHT11_DATA_PIN) != 0) != high) {
        if ((uint32_t)(time_us_32() - start_us) >= DHT11_TIMEOUT_US) {
            return false;
        }
    }
    return true;
}

static bool dht11_read(float *temperature_c, float *humidity_percent) {
    uint8_t data[5] = {0};

    gpio_set_dir(DHT11_DATA_PIN, GPIO_OUT);
    gpio_put(DHT11_DATA_PIN, 0);
    sleep_ms(18);
    gpio_put(DHT11_DATA_PIN, 1);
    sleep_us(30);
    gpio_set_dir(DHT11_DATA_PIN, GPIO_IN);
    gpio_pull_up(DHT11_DATA_PIN);

    if (!dht11_wait_for_level(false) ||
        !dht11_wait_for_level(true) ||
        !dht11_wait_for_level(false)) {
        return false;
    }

    for (size_t bit = 0; bit < 40; ++bit) {
        if (!dht11_wait_for_level(true)) {
            return false;
        }
        uint32_t pulse_start_us = time_us_32();
        if (!dht11_wait_for_level(false)) {
            return false;
        }
        uint32_t pulse_width_us = time_us_32() - pulse_start_us;
        if (pulse_width_us < 10u) {
            return false;
        }
        data[bit / 8] <<= 1;
        if (pulse_width_us >= DHT11_ONE_THRESHOLD_US) {
            data[bit / 8] |= 1u;
        }
    }

    if ((uint8_t)(data[0] + data[1] + data[2] + data[3]) != data[4] ||
        data[0] > 100u || data[1] > 9u || data[2] > 80u || data[3] > 9u) {
        return false;
    }

    *humidity_percent = (float)data[0] + (float)data[1] / 10.0f;
    *temperature_c = (float)data[2] + (float)data[3] / 10.0f;
    return true;
}

static float calibrate_current_zero_raw(void) {
    double mean = 0.0;
    double sum_squared_deviation = 0.0;

    adc_select_input(CURRENT_ADC_INPUT);
    (void)adc_read();
    absolute_time_t next_sample = get_absolute_time();
    for (unsigned int i = 0; i < ADC_RMS_SAMPLE_COUNT; ++i) {
        double sample = adc_read();
        double deviation = sample - mean;
        mean += deviation / (i + 1);
        sum_squared_deviation += deviation * (sample - mean);
        if (i + 1 < ADC_RMS_SAMPLE_COUNT) {
            wait_for_next_adc_sample(&next_sample);
        }
    }
    current_zero_noise_rms_raw =
        (float)sqrt(sum_squared_deviation / ADC_RMS_SAMPLE_COUNT);
    return (float)mean;
}

static float read_rms_voltage(uint input, bool remove_mean,
                              float zero_point_raw) {
    double mean = 0.0;
    double square_sum = 0.0;

    adc_select_input(input);
    (void)adc_read();
    absolute_time_t next_sample = get_absolute_time();
    for (unsigned int i = 0; i < ADC_RMS_SAMPLE_COUNT; ++i) {
        float sample = (float)adc_read();
        if (remove_mean) {
            double deviation_from_mean = sample - mean;
            mean += deviation_from_mean / (i + 1);
            square_sum += deviation_from_mean * (sample - mean);
        } else {
            double deviation_from_zero = sample - zero_point_raw;
            square_sum += deviation_from_zero * deviation_from_zero;
        }
        if (i + 1 < ADC_RMS_SAMPLE_COUNT) {
            wait_for_next_adc_sample(&next_sample);
        }
    }
    return adc_raw_to_volts(
        (float)sqrt(square_sum / ADC_RMS_SAMPLE_COUNT));
}

static float correct_current_sensor_rms_voltage(float current_rms_voltage) {
    float current_rms_raw = current_rms_voltage * (4095.0f / 3.3f);
    float noise_threshold_raw =
        fmaxf(1.0f, CURRENT_ZERO_NOISE_MULTIPLIER *
                         current_zero_noise_rms_raw);
    if (current_rms_raw <= noise_threshold_raw) {
        return 0.0f;
    }

    float corrected_rms_raw = sqrtf(
        current_rms_raw * current_rms_raw -
        current_zero_noise_rms_raw * current_zero_noise_rms_raw);
    return adc_raw_to_volts(corrected_rms_raw) * CURRENT_ADC_DIVIDER_RATIO;
}

static float read_current_sensor_rms_voltage(void) {
    return correct_current_sensor_rms_voltage(read_rms_voltage(
        CURRENT_ADC_INPUT, false, current_zero_current_raw));
}

static bool add_current_calibration_point(float reference_current_a) {
    if (!isfinite(reference_current_a) ||
        reference_current_a <= 0.0f || reference_current_a > 30.0f) {
        printf("Calibration current must be greater than 0 and at most 30 A\n");
        return false;
    }
    if (current_calibration_point_count >= CURRENT_CALIBRATION_POINT_COUNT) {
        printf("Calibration point limit reached; use K RESET to start again\n");
        return false;
    }

    float sensor_rms_voltage_v = read_current_sensor_rms_voltage();
    if (sensor_rms_voltage_v <= 0.0f) {
        printf("Calibration rejected: measured current is within the zero/noise threshold\n");
        return false;
    }

    current_calibration_points[current_calibration_point_count].sensor_rms_voltage_v =
        sensor_rms_voltage_v;
    current_calibration_points[current_calibration_point_count].reference_current_a =
        reference_current_a;
    ++current_calibration_point_count;

    double voltage_squared_sum = 0.0;
    double voltage_current_sum = 0.0;
    for (size_t i = 0; i < current_calibration_point_count; ++i) {
        double voltage =
            current_calibration_points[i].sensor_rms_voltage_v;
        double current =
            current_calibration_points[i].reference_current_a;
        voltage_squared_sum += voltage * voltage;
        voltage_current_sum += voltage * current;
    }
    if (!isfinite(voltage_squared_sum) ||
        !isfinite(voltage_current_sum) || voltage_current_sum <= 0.0) {
        --current_calibration_point_count;
        printf("Calibration rejected: invalid calibration data\n");
        return false;
    }

    current_sensitivity_v_per_a =
        (float)(voltage_squared_sum / voltage_current_sum);
    printf("Current calibration point %u: reference %.3f A, sensor %.4f Vrms; "
           "sensitivity %.5f V/A (%u point%s)\n",
           (unsigned int)current_calibration_point_count,
           reference_current_a, sensor_rms_voltage_v,
           current_sensitivity_v_per_a,
           (unsigned int)current_calibration_point_count,
           current_calibration_point_count == 1 ? "" : "s");
    return true;
}

static float read_soil_percent(void) {
    uint32_t total = 0;
    adc_select_input(SOIL_ADC_INPUT);
    for (unsigned int i = 0; i < 16; ++i) {
        total += adc_read();
    }
    float raw = (float)total / 16.0f;
    float percent = ((float)SOIL_DRY_ADC_RAW - raw) * 100.0f /
                    ((float)SOIL_DRY_ADC_RAW - (float)SOIL_WET_ADC_RAW);
    if (percent < 0.0f) {
        percent = 0.0f;
    } else if (percent > 100.0f) {
        percent = 100.0f;
    }
    return percent;
}

static void update_sensor_alerts(void) {
    bool high_current = readings.current_a >= overcurrent_threshold_a;
    if (high_current && !overcurrent_active) {
        append_alert("HIGH_CURRENT");
        printf("ALERT: high current %.2f A (limit %.2f A)\n",
               readings.current_a, overcurrent_threshold_a);
    }
    if (high_current) {
        set_relay(1, false, "overcurrent safety");
    }
    overcurrent_active = high_current;

    bool low_voltage = readings.voltage_v < low_voltage_threshold_v;
    if (low_voltage && !low_voltage_active) {
        append_alert("LOW_VOLTAGE");
        printf("ALERT: low voltage %.1f V (limit %.1f V)\n",
               readings.voltage_v, low_voltage_threshold_v);
    }
    low_voltage_active = low_voltage;

    bool soil_low = readings.soil_percent < soil_low_threshold_percent;
    bool soil_high = readings.soil_percent > soil_high_threshold_percent;
    if (soil_low && !soil_low_active) {
        append_alert("SOIL_LOW");
        printf("ALERT: low soil moisture %.1f%%\n", readings.soil_percent);
    }
    if (soil_high && !soil_high_active) {
        append_alert("SOIL_HIGH");
        printf("ALERT: high soil moisture %.1f%%\n", readings.soil_percent);
    }
    soil_low_active = soil_low;
    soil_high_active = soil_high;
}

static void begin_sensor_sampling(sensor_sampling_phase_t phase, uint input) {
    sensor_sampler.phase = phase;
    sensor_sampler.sample_count = 0;
    sensor_sampler.mean = 0.0;
    sensor_sampler.square_sum = 0.0;
    adc_select_input(input);
    (void)adc_read();
    sensor_sampler.next_sample_us =
        time_us_64() + ADC_SAMPLE_PERIOD_US;
}

static void start_sensor_update(void) {
    if (sensor_sampler.phase == SENSOR_SAMPLING_IDLE) {
        begin_sensor_sampling(SENSOR_SAMPLING_VOLTAGE, VOLTAGE_ADC_INPUT);
    }
}

static void service_sensor_update(void) {
    if (sensor_sampler.phase == SENSOR_SAMPLING_IDLE) {
        return;
    }

    uint64_t now = time_us_64();
    if (now < sensor_sampler.next_sample_us) {
        return;
    }
    if (now - sensor_sampler.next_sample_us >
        (uint64_t)ADC_SAMPLE_PERIOD_US * 2) {
        uint input = sensor_sampler.phase == SENSOR_SAMPLING_VOLTAGE
                         ? VOLTAGE_ADC_INPUT
                         : CURRENT_ADC_INPUT;
        begin_sensor_sampling(sensor_sampler.phase, input);
        return;
    }

    double sample = adc_read();
    if (sensor_sampler.phase == SENSOR_SAMPLING_VOLTAGE) {
        double deviation = sample - sensor_sampler.mean;
        sensor_sampler.mean += deviation / (sensor_sampler.sample_count + 1);
        sensor_sampler.square_sum +=
            deviation * (sample - sensor_sampler.mean);
    } else {
        double deviation = sample - current_zero_current_raw;
        sensor_sampler.square_sum += deviation * deviation;
    }
    ++sensor_sampler.sample_count;

    if (sensor_sampler.sample_count < ADC_RMS_SAMPLE_COUNT) {
        sensor_sampler.next_sample_us += ADC_SAMPLE_PERIOD_US;
        return;
    }

    float rms_voltage = adc_raw_to_volts((float)sqrt(
        sensor_sampler.square_sum / ADC_RMS_SAMPLE_COUNT));
    if (sensor_sampler.phase == SENSOR_SAMPLING_VOLTAGE) {
        sensor_sampler.voltage_sensor_rms_v = rms_voltage;
        sensor_sampler.voltage_v =
            rms_voltage * voltage_calibration_v_per_adc_v;
        begin_sensor_sampling(SENSOR_SAMPLING_CURRENT, CURRENT_ADC_INPUT);
        return;
    }

    float current_sensor_rms =
        correct_current_sensor_rms_voltage(rms_voltage);
    sensor_sampler.current_a =
        current_sensor_rms / current_sensitivity_v_per_a;
    readings.voltage_v = sensor_sampler.voltage_v;
    readings.current_a = sensor_sampler.current_a;
    readings.soil_percent = read_soil_percent();
    sensor_sampler.phase = SENSOR_SAMPLING_IDLE;
    sensor_readings_valid = true;
    update_sensor_alerts();
}

static void update_dht11(void) {
    static bool error_reported;
    float temperature_c;
    float humidity_percent;
    if (dht11_read(&temperature_c, &humidity_percent)) {
        readings.temperature_c = temperature_c;
        readings.humidity_percent = humidity_percent;
        readings.dht11_valid = true;
        error_reported = false;
    } else {
        readings.dht11_valid = false;
        if (!error_reported) {
            printf("DHT11 read failed; temperature and humidity will be reported as unavailable\n");
            error_reported = true;
        }
    }
}

static void acknowledge_transmitted_alerts(void) {
    size_t sent_length = strlen(transmitting_alerts);
    if (sent_length == 0 ||
        strncmp(pending_alerts, transmitting_alerts, sent_length) != 0) {
        return;
    }

    if (pending_alerts[sent_length] == '\0') {
        pending_alerts[0] = '\0';
    } else if (pending_alerts[sent_length] == '|') {
        size_t remaining_length = strlen(pending_alerts + sent_length + 1);
        memmove(pending_alerts, pending_alerts + sent_length + 1,
                remaining_length + 1);
    }
}

static bool calibrate_voltage(float reference_voltage_v) {
    if (!isfinite(reference_voltage_v) ||
        reference_voltage_v < 1.0f || reference_voltage_v > 500.0f) {
        printf("Voltage calibration reference must be 1-500 V RMS\n");
        return false;
    }
    if (!sensor_readings_valid) {
        printf("Voltage calibration unavailable: wait for a complete sensor reading\n");
        return false;
    }
    if (!isfinite(sensor_sampler.voltage_sensor_rms_v) ||
        sensor_sampler.voltage_sensor_rms_v < 0.01f) {
        printf("Voltage calibration rejected: ZMPT ADC RMS is too small; check wiring and module gain\n");
        return false;
    }

    float calibration =
        reference_voltage_v / sensor_sampler.voltage_sensor_rms_v;
    if (!isfinite(calibration) || calibration <= 0.0f) {
        printf("Voltage calibration rejected: invalid calibration factor\n");
        return false;
    }

    voltage_calibration_v_per_adc_v = calibration;
    printf("Voltage calibration set: reference %.2f V RMS, ADC %.4f V RMS, "
           "factor %.3f V/ADC-V\n",
           reference_voltage_v, sensor_sampler.voltage_sensor_rms_v,
           voltage_calibration_v_per_adc_v);
    return true;
}

static void process_relay_command(char *command) {
    for (char *ch = command; *ch != '\0'; ++ch) {
        *ch = (char)toupper((unsigned char)*ch);
        if (*ch == '=') {
            *ch = ' ';
        }
    }

    char *save_pointer = NULL;
    for (char *part = strtok_r(command, ",;", &save_pointer);
         part != NULL;
         part = strtok_r(NULL, ",;", &save_pointer)) {
        int relay_number = 0;
        char state[8] = "";
        if (sscanf(part, " R%d %7s", &relay_number, state) != 2 &&
            sscanf(part, " RELAY%d %7s", &relay_number, state) != 2) {
            printf("Ignored invalid LoRa command: %s\n", part);
            continue;
        }
        if ((relay_number != 1 && relay_number != 2) ||
            (strcmp(state, "ON") != 0 && strcmp(state, "OFF") != 0)) {
            printf("Ignored invalid relay command: %s\n", part);
            continue;
        }
        bool turn_on = strcmp(state, "ON") == 0;
        if (relay_number == 1 && turn_on &&
            (!sensor_readings_valid ||
             readings.current_a >= overcurrent_threshold_a)) {
            if (!sensor_readings_valid) {
                printf("Relay 1 ON rejected: current reading is not ready\n");
                append_alert("RELAY1_ON_BLOCKED");
                continue;
            }
            printf("Relay 1 ON rejected: current %.2f A exceeds the %.2f A limit\n",
                   readings.current_a, overcurrent_threshold_a);
            append_alert("RELAY1_ON_BLOCKED");
            continue;
        }
        set_relay((unsigned int)relay_number, turn_on, "LoRa command");
    }
}

static void process_serial_command(char *command) {
    unsigned long unsigned_value;
    int integer_value;
    float float_value;

    if (sscanf(command, "F %lu", &unsigned_value) == 1) {
        if (unsigned_value >= 137000000UL && unsigned_value <= 1020000000UL) {
            frequency_hz = (uint32_t)unsigned_value;
            ra02_apply_configuration();
            printf("Frequency set to %lu Hz\n", (unsigned long)frequency_hz);
        } else {
            printf("Invalid frequency\n");
        }
    } else if (sscanf(command, "S %d", &integer_value) == 1) {
        if (integer_value >= 7 && integer_value <= 12) {
            spreading_factor = (uint8_t)integer_value;
            ra02_apply_configuration();
            printf("Spreading factor set to %d\n", integer_value);
        } else {
            printf("Invalid SF (7-12)\n");
        }
    } else if (sscanf(command, "B %d", &integer_value) == 1) {
        if (integer_value == 125 || integer_value == 250 || integer_value == 500) {
            bandwidth_hz = (uint32_t)integer_value * 1000u;
            ra02_apply_configuration();
            printf("Bandwidth set to %d kHz\n", integer_value);
        } else {
            printf("Invalid BW (125/250/500 kHz)\n");
        }
    } else if (sscanf(command, "P %d", &integer_value) == 1) {
        if (integer_value >= 2 && integer_value <= 17) {
            tx_power_dbm = (uint8_t)integer_value;
            ra02_apply_configuration();
            printf("TX power set to %d dBm\n", integer_value);
        } else {
            printf("Invalid power (2-17 dBm)\n");
        }
    } else if (sscanf(command, "C %d", &integer_value) == 1) {
        if (integer_value >= 5 && integer_value <= 8) {
            coding_rate = (uint8_t)integer_value;
            ra02_apply_configuration();
            printf("Coding rate set to 4/%d\n", integer_value);
        } else {
            printf("Invalid coding rate (5-8)\n");
        }
    } else if (sscanf(command, "I %lu", &unsigned_value) == 1) {
        if (unsigned_value >= 1000 && unsigned_value <= 3600000) {
            telemetry_interval_ms = (uint32_t)unsigned_value;
            printf("Telemetry interval set to %lu ms\n", unsigned_value);
        } else {
            printf("Invalid interval (1000-3600000 ms)\n");
        }
    } else if (strncmp(command, "Z ", 2) == 0) {
        const char *calibration_command = command + 2;
        if (strcmp(calibration_command, "RESET") == 0) {
            voltage_calibration_v_per_adc_v =
                VOLTAGE_CALIBRATION_V_PER_ADC_V;
            printf("Voltage calibration reset to default factor %.3f V/ADC-V\n",
                   voltage_calibration_v_per_adc_v);
        } else if (sscanf(calibration_command, "%f", &float_value) == 1) {
            (void)calibrate_voltage(float_value);
        } else {
            printf("Usage: Z <known-voltage-RMS>, or Z RESET\n");
        }
    } else if (strncmp(command, "K ", 2) == 0) {
        const char *calibration_command = command + 2;
        if (strcmp(calibration_command, "ZERO") == 0) {
            current_zero_current_raw = calibrate_current_zero_raw();
            current_calibration_point_count = 0;
            current_sensitivity_v_per_a = ACS712_SENSITIVITY_V_PER_A;
            printf("Current zero recalibrated: %.3f V, idle noise RMS: %.2f mV; "
                   "sensitivity reset to nominal %.5f V/A. Ensure no current "
                   "is flowing\n",
                   adc_raw_to_volts(current_zero_current_raw) *
                       CURRENT_ADC_DIVIDER_RATIO,
                   adc_raw_to_volts(current_zero_noise_rms_raw) *
                       CURRENT_ADC_DIVIDER_RATIO * 1000.0f,
                   current_sensitivity_v_per_a);
        } else if (strcmp(calibration_command, "RESET") == 0) {
            current_calibration_point_count = 0;
            current_sensitivity_v_per_a = ACS712_SENSITIVITY_V_PER_A;
            printf("Current sensitivity reset to nominal %.5f V/A\n",
                   current_sensitivity_v_per_a);
        } else if (sscanf(calibration_command, "%f", &float_value) == 1) {
            (void)add_current_calibration_point(float_value);
        } else {
            printf("Usage: K <known-current-A>, K ZERO, or K RESET\n");
        }
    } else if (sscanf(command, "H %f", &float_value) == 1) {
        if (float_value > 0.0f && float_value <= 30.0f) {
            overcurrent_threshold_a = float_value;
            printf("Overcurrent limit set to %.2f A\n", overcurrent_threshold_a);
        } else {
            printf("Invalid current limit (0-30 A)\n");
        }
    } else if (sscanf(command, "V %f", &float_value) == 1) {
        if (float_value >= 0.0f && float_value <= 500.0f) {
            low_voltage_threshold_v = float_value;
            printf("Low voltage threshold set to %.1f V\n", low_voltage_threshold_v);
        } else {
            printf("Invalid voltage threshold (0-500 V)\n");
        }
    } else if (sscanf(command, "L %f", &float_value) == 1) {
        if (float_value >= 0.0f && float_value < soil_high_threshold_percent) {
            soil_low_threshold_percent = float_value;
            printf("Low soil threshold set to %.1f%%\n", soil_low_threshold_percent);
        } else {
            printf("Invalid low soil threshold\n");
        }
    } else if (sscanf(command, "U %f", &float_value) == 1) {
        if (float_value <= 100.0f && float_value > soil_low_threshold_percent) {
            soil_high_threshold_percent = float_value;
            printf("High soil threshold set to %.1f%%\n", soil_high_threshold_percent);
        } else {
            printf("Invalid high soil threshold\n");
        }
    } else if (strncmp(command, "T ", 2) == 0) {
        size_t name_length = strlen(command + 2);
        if (name_length > 0 && name_length < sizeof(node_name)) {
            memcpy(node_name, command + 2, name_length + 1);
            printf("Node name set to %s\n", node_name);
        } else {
            printf("Node name must be 1-23 characters\n");
        }
    } else if (strcmp(command, "?") == 0 || strcmp(command, "help") == 0) {
        printf("Serial commands:\n"
               "  F <Hz>       Set frequency\n"
               "  S <7-12>     Set spreading factor\n"
               "  B <125|250|500> Set bandwidth in kHz\n"
               "  P <2-17>     Set TX power in dBm\n"
               "  C <5-8>      Set coding rate (4/x)\n"
               "  I <ms>       Set telemetry interval (1000-3600000)\n"
               "  Z <volts>    Calibrate ZMPT101B with known AC RMS voltage\n"
               "  Z RESET      Restore default ZMPT voltage calibration\n"
               "  K <amps>     Add a known-current calibration point\n"
               "  K ZERO       Recalibrate zero with no current flowing\n"
               "  K RESET      Clear calibration points; restore nominal sensitivity\n"
               "  H <amps>     Set relay-1 overcurrent limit\n"
               "  V <volts>    Set low-voltage alert threshold\n"
               "  L <percent>  Set low soil moisture threshold\n"
               "  U <percent>  Set high soil moisture threshold\n"
               "  T <name>     Set telemetry node name\n"
               "LoRa commands: R1 ON, R1 OFF, R2 ON, R2 OFF\n");
    } else {
        printf("Unknown command. Type ? for help.\n");
    }
}

static void poll_serial_commands(char *buffer, size_t buffer_size, size_t *index) {
    int character = getchar_timeout_us(0);
    if (character == PICO_ERROR_TIMEOUT) {
        return;
    }
    if (character == '\n' || character == '\r') {
        if (*index > 0) {
            buffer[*index] = '\0';
            process_serial_command(buffer);
            *index = 0;
        }
    } else if (*index < buffer_size - 1) {
        buffer[(*index)++] = (char)character;
    }
}

static void print_config(void) {
    printf("\n=== RP2040 LoRa Sensor/Relay Node ===\n");
    printf("Frequency: %lu Hz, SF%d, BW%lu kHz, CR4/%d, TX %d dBm\n",
           (unsigned long)frequency_hz, spreading_factor,
           (unsigned long)(bandwidth_hz / 1000), coding_rate, tx_power_dbm);
    printf("Telemetry: %lu ms, relay-1 current limit: %.2f A\n",
           (unsigned long)telemetry_interval_ms, overcurrent_threshold_a);
    printf("ZMPT voltage calibration factor: %.3f V/ADC-V\n",
           voltage_calibration_v_per_adc_v);
    printf("Analog inputs: GP26 voltage, GP27 current, GP28 soil\n");
    printf("DHT11 data input: GP%u\n", (unsigned int)DHT11_DATA_PIN);
    printf("Relay outputs: GP2 relay 1, GP3 relay 2 (active-high)\n");
    printf("Sensor calibration defaults are in the source configuration section.\n\n");
}

int main(void) {
    stdio_init_all();
    sleep_ms(1500);

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, 0);

    gpio_init(PIN_CS);
    gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_put(PIN_CS, 1);
    gpio_init(PIN_RESET);
    gpio_set_dir(PIN_RESET, GPIO_OUT);
    gpio_put(PIN_RESET, 1);
    gpio_init(PIN_DIO0);
    gpio_set_dir(PIN_DIO0, GPIO_IN);

    gpio_init(RELAY_1_PIN);
    gpio_set_dir(RELAY_1_PIN, GPIO_OUT);
    gpio_put(RELAY_1_PIN, !RELAY_ACTIVE_LEVEL);
    gpio_init(RELAY_2_PIN);
    gpio_set_dir(RELAY_2_PIN, GPIO_OUT);
    gpio_put(RELAY_2_PIN, !RELAY_ACTIVE_LEVEL);

    gpio_init(DHT11_DATA_PIN);
    gpio_set_dir(DHT11_DATA_PIN, GPIO_IN);
    gpio_pull_up(DHT11_DATA_PIN);

    spi_init(SPI_PORT, SPI_BAUD);
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);

    adc_init();
    adc_gpio_init(26 + VOLTAGE_ADC_INPUT);
    adc_gpio_init(26 + CURRENT_ADC_INPUT);
    adc_gpio_init(26 + SOIL_ADC_INPUT);
    current_zero_current_raw = calibrate_current_zero_raw();
    printf("Current zero: %.3f V, idle noise RMS: %.2f mV, sensitivity: %.5f V/A\n",
           adc_raw_to_volts(current_zero_current_raw) *
               CURRENT_ADC_DIVIDER_RATIO,
           adc_raw_to_volts(current_zero_noise_rms_raw) *
               CURRENT_ADC_DIVIDER_RATIO * 1000.0f,
           current_sensitivity_v_per_a);

    ra02_reset();
    sleep_ms(50);
    uint8_t version = ra02_read_reg(REG_VERSION);
    printf("SX127x version: 0x%02X\n", version);
    if (version != 0x12) {
        printf("Unexpected LoRa chip version; check SPI wiring and radio power.\n");
        while (true) {
            sleep_ms(1000);
        }
    }

    ra02_configure_lora();
    print_config();
    printf("UART0 debug output: GP0 TX, GP1 RX, 115200 baud, 8-N-1.\n");
    printf("LoRa settings: frequency=%lu Hz, SF%u, BW=%lu Hz, CR=4/%u, sync=0x12, preamble=12.\n",
           (unsigned long)frequency_hz, spreading_factor,
           (unsigned long)(bandwidth_hz), coding_rate);
    printf("Type ? on USB serial for configuration. LoRa receiver is active between telemetry packets.\n");

    char serial_buffer[MAX_COMMAND_LENGTH + 1];
    size_t serial_index = 0;
    char rx_buffer[MAX_PAYLOAD_LENGTH + 1];
    uint64_t now = time_us_64();
    uint64_t last_sensor_update_us = now;
    uint64_t last_dht11_update_us = 0;
    uint64_t last_telemetry_us = now;

    start_sensor_update();
    last_sensor_update_us = time_us_64();
    ra02_start_rx();

    while (true) {
        poll_serial_commands(serial_buffer, sizeof(serial_buffer), &serial_index);

        now = time_us_64();
        if (now - last_sensor_update_us >=
            (uint64_t)SENSOR_SAMPLE_INTERVAL_MS * 1000) {
            last_sensor_update_us = now;
            start_sensor_update();
        }

        service_sensor_update();

        now = time_us_64();
        if (sensor_sampler.phase == SENSOR_SAMPLING_IDLE &&
            now - last_dht11_update_us >=
            (uint64_t)DHT11_SAMPLE_INTERVAL_MS * 1000) {
            last_dht11_update_us = now;
            update_dht11();
        }

        if (radio_tx_active) {
            radio_tx_status_t tx_status = ra02_poll_packet_transmit();
            if (tx_status != RADIO_TX_PENDING) {
                radio_tx_active = false;
                if (tx_status == RADIO_TX_SUCCEEDED) {
                    gpio_put(LED_PIN, 1);
                    printf("[LoRa TX] complete\n");
                    gpio_put(LED_PIN, 0);
                    acknowledge_transmitted_alerts();
                } else {
                    printf("[LoRa TX] failed (timeout)\n");
                }
                if (radio_configuration_pending) {
                    radio_configuration_pending = false;
                    ra02_apply_configuration();
                } else {
                    ra02_start_rx();
                }
                last_telemetry_us = time_us_64();
            }
        } else {
            size_t received_length;
            if (ra02_receive_packet(rx_buffer, sizeof(rx_buffer), &received_length)) {
                debug_print_packet("RX", (const uint8_t *)rx_buffer, received_length);
                process_relay_command(rx_buffer);
                ra02_start_rx();
            }
        }

        now = time_us_64();
        if (!radio_tx_active &&
            now - last_telemetry_us >= (uint64_t)telemetry_interval_ms * 1000) {
            char packet[MAX_PAYLOAD_LENGTH + 1];
            const char *alerts = pending_alerts[0] == '\0' ? "NONE" : pending_alerts;
            int alert_length = snprintf(transmitting_alerts,
                                        sizeof(transmitting_alerts), "%s", alerts);
            char temperature[16];
            char humidity[16];
            if (readings.dht11_valid) {
                (void)snprintf(temperature, sizeof(temperature), "%.1fC",
                               readings.temperature_c);
                (void)snprintf(humidity, sizeof(humidity), "%.1f%%",
                               readings.humidity_percent);
            } else {
                (void)snprintf(temperature, sizeof(temperature), "NA");
                (void)snprintf(humidity, sizeof(humidity), "NA");
            }
            int packet_length = snprintf(
                packet, sizeof(packet),
                "node=%s,v=%.1fV,i=%.2fA,soil=%.1f%%,temp=%s,humidity=%s,r1=%s,r2=%s,alert=%s",
                node_name, readings.voltage_v, readings.current_a,
                readings.soil_percent, temperature, humidity,
                relay_1_on ? "ON" : "OFF",
                relay_2_on ? "ON" : "OFF", alerts);
            if (alert_length >= 0 &&
                (size_t)alert_length < sizeof(transmitting_alerts) &&
                packet_length > 0 && (size_t)packet_length < sizeof(packet)) {
                debug_print_packet("TX", (const uint8_t *)packet,
                                   (size_t)packet_length);
                radio_tx_active = ra02_begin_packet_transmit(
                    (const uint8_t *)packet, (size_t)packet_length);
                if (!radio_tx_active) {
                    printf("[LoRa TX] failed to start\n");
                    last_telemetry_us = time_us_64();
                }
            } else {
                printf("[LoRa TX] invalid formatted packet length=%d (capacity=%u)\n",
                       packet_length, (unsigned int)sizeof(packet));
                last_telemetry_us = time_us_64();
            }
        }
        if (sensor_sampler.phase == SENSOR_SAMPLING_IDLE) {
            sleep_ms(5);
        } else {
            sleep_us(50);
        }
    }
}
