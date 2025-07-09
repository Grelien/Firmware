idf_component_register(
SRCS "time_util.c" "webserver.c" "wifi_ap.c" "storage.c" "schedule.c" "motor_control.c" "motor_controller.c"
                      INCLUDE_DIRS "."
    EMBED_FILES "logo.png"
    REQUIRES 
        esp_wifi
        esp_event
        esp_netif
        esp_http_server
        nvs_flash
        driver
        esp_timer)
        
target_add_binary_data(${COMPONENT_TARGET} "logo.png" BINARY)