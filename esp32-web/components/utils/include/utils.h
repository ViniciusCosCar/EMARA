#include "esp_log.h"

#define LOGFI(tag, fmt, ...) ESP_LOGI(tag, "[%s:%d] " fmt, __func__, __LINE__, ##__VA_ARGS__)
#define LOGFE(tag, fmt, ...) ESP_LOGE(tag, "[%s:%d] " fmt, __func__, __LINE__, ##__VA_ARGS__)
