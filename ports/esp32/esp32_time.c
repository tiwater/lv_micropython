#include "esp32_time.h"
#include "nvs.h"
#include "esp_err.h"

static bool s_up_to_date;
static int8_t s_timezone;
const static char* NVS_NAMESPACE = "TM";
const static char* NVS_KEY = "TZ";

static bool _nvs_write_tz(int8_t tz)
{
    nvs_handle_t handle;
    esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &handle);
    if (ESP_OK == err)
    {
        err = nvs_set_i8(handle, NVS_KEY, tz);
        nvs_commit(handle);
        nvs_close(handle);
    }
    return ESP_OK == err;
}

static bool _nvs_read_tz(int8_t* tz)
{
    nvs_handle_t handle;
    esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READONLY, &handle);
    if (ESP_OK == err)
    {
        err = nvs_get_i8(handle, NVS_KEY, tz);
        nvs_close(handle);
    }
    return ESP_OK == err;
}

bool mod_settimezone(int8_t tz)
{
    bool rt = _nvs_write_tz(tz);
    s_up_to_date = false;
    return rt;
}

int8_t mod_gettimezone(void)
{
    if (!s_up_to_date)
    {
        s_up_to_date = _nvs_read_tz(&s_timezone);
    }
    return s_timezone;
}

bool mod_settimestamp(time_t ts)
{
    struct timeval tv = { ts, 0 };
    return settimeofday(&tv, NULL) == 0;
}

time_t mod_gettimestamp(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec;
}
