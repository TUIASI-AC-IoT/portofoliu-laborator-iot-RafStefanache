/*  WiFi softAP Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "freertos/event_groups.h"
#include "esp_http_server.h"


#include "lwip/err.h"
#include "lwip/sys.h"

#include "soft-ap.h"
#include "http-server.h"

#include "../mdns/include/mdns.h"
#include <string.h>

#define CONFIG_ESP_WIFI_SSID      "lab-iot"
#define CONFIG_ESP_WIFI_PASS      "IoT-IoT-IoT"
#define CONFIG_ESP_MAXIMUM_RETRY  5
#define CONFIG_LOCAL_PORT         10001
#define DEFAULT_SCAN_LIST_SIZE 10

static void wifi_scan(void);
static const char *TAG = "wifi station";

void app_main(void)
{
    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // TODO: 3. Pornire mod STA + scanare SSID-uri disponibile
    wifi_scan();

    // TODO: 4. Initializare mDNS (daca mai ramana timp)    
    ESP_ERROR_CHECK(mdns_init());
    mdns_hostname_set("esp32-rafael");
    mdns_instance_name_set("ESP32 SoftAP Web Server");

    // TODO: 1. Pornire softAP
    wifi_init_softap();

    // TODO: 2. Pornire server web (si config specifice in http-server.c) 


    static httpd_handle_t server = NULL;
    server = start_webserver();
}


static void print_auth_mode(int authmode)
{
    switch (authmode) {
    case WIFI_AUTH_OPEN:
        ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_OPEN");
        break;
    case WIFI_AUTH_WEP:
        ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_WEP");
        break;
    case WIFI_AUTH_WPA_PSK:
        ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_WPA_PSK");
        break;
    case WIFI_AUTH_WPA2_PSK:
        ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_WPA2_PSK");
        break;
    case WIFI_AUTH_WPA_WPA2_PSK:
        ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_WPA_WPA2_PSK");
        break;
    case WIFI_AUTH_WPA2_ENTERPRISE:
        ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_WPA2_ENTERPRISE");
        break;
    case WIFI_AUTH_WPA3_PSK:
        ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_WPA3_PSK");
        break;
    case WIFI_AUTH_WPA2_WPA3_PSK:
        ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_WPA2_WPA3_PSK");
        break;
    default:
        ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_UNKNOWN");
        break;
    }
}


static void print_cipher_type(int pairwise_cipher, int group_cipher)
{
    switch (pairwise_cipher) {
    case WIFI_CIPHER_TYPE_NONE:
        ESP_LOGI(TAG, "Pairwise Cipher \tWIFI_CIPHER_TYPE_NONE");
        break;
    case WIFI_CIPHER_TYPE_WEP40:
        ESP_LOGI(TAG, "Pairwise Cipher \tWIFI_CIPHER_TYPE_WEP40");
        break;
    case WIFI_CIPHER_TYPE_WEP104:
        ESP_LOGI(TAG, "Pairwise Cipher \tWIFI_CIPHER_TYPE_WEP104");
        break;
    case WIFI_CIPHER_TYPE_TKIP:
        ESP_LOGI(TAG, "Pairwise Cipher \tWIFI_CIPHER_TYPE_TKIP");
        break;
    case WIFI_CIPHER_TYPE_CCMP:
        ESP_LOGI(TAG, "Pairwise Cipher \tWIFI_CIPHER_TYPE_CCMP");
        break;
    case WIFI_CIPHER_TYPE_TKIP_CCMP:
        ESP_LOGI(TAG, "Pairwise Cipher \tWIFI_CIPHER_TYPE_TKIP_CCMP");
        break;
    default:
        ESP_LOGI(TAG, "Pairwise Cipher \tWIFI_CIPHER_TYPE_UNKNOWN");
        break;
    }

    switch (group_cipher) {
    case WIFI_CIPHER_TYPE_NONE:
        ESP_LOGI(TAG, "Group Cipher \tWIFI_CIPHER_TYPE_NONE");
        break;
    case WIFI_CIPHER_TYPE_WEP40:
        ESP_LOGI(TAG, "Group Cipher \tWIFI_CIPHER_TYPE_WEP40");
        break;
    case WIFI_CIPHER_TYPE_WEP104:
        ESP_LOGI(TAG, "Group Cipher \tWIFI_CIPHER_TYPE_WEP104");
        break;
    case WIFI_CIPHER_TYPE_TKIP:
        ESP_LOGI(TAG, "Group Cipher \tWIFI_CIPHER_TYPE_TKIP");
        break;
    case WIFI_CIPHER_TYPE_CCMP:
        ESP_LOGI(TAG, "Group Cipher \tWIFI_CIPHER_TYPE_CCMP");
        break;
    case WIFI_CIPHER_TYPE_TKIP_CCMP:
        ESP_LOGI(TAG, "Group Cipher \tWIFI_CIPHER_TYPE_TKIP_CCMP");
        break;
    default:
        ESP_LOGI(TAG, "Group Cipher \tWIFI_CIPHER_TYPE_UNKNOWN");
        break;
    }
}

static void wifi_scan(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
    assert(sta_netif);

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    uint16_t number = DEFAULT_SCAN_LIST_SIZE;
    wifi_ap_record_t ap_info[DEFAULT_SCAN_LIST_SIZE];
    uint16_t ap_count = 0;
    memset(ap_info, 0, sizeof(ap_info));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
    esp_wifi_scan_start(NULL, true);
    ESP_LOGI(TAG, "Scanning for available Wi-Fi networks...");
    
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&number, ap_info));
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&ap_count));
    ESP_LOGI(TAG, "Total APs scanned = %u", ap_count);
    
    for (int i = 0; i < number; i++) {
        ESP_LOGI(TAG, "SSID: %s, RSSI: %d", ap_info[i].ssid, ap_info[i].rssi);
        print_auth_mode(ap_info[i].authmode);
    }
}



/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifi_event_group;

/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1


static int s_retry_num = 0;

static void event_handler(void* arg, esp_event_base_t event_base,
    int32_t event_id, void* event_data)
{
if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
esp_wifi_connect();
} else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
if (s_retry_num < CONFIG_ESP_MAXIMUM_RETRY) {
esp_wifi_connect();
s_retry_num++;
ESP_LOGI(TAG, "retry to connect to the AP");
} else {
xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
}
ESP_LOGI(TAG,"connect to the AP fail");
} else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
s_retry_num = 0;
xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
}
}

bool wifi_init_sta(void)
{
s_wifi_event_group = xEventGroupCreate();

ESP_ERROR_CHECK(esp_netif_init());

ESP_ERROR_CHECK(esp_event_loop_create_default());
esp_netif_create_default_wifi_sta();

wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
ESP_ERROR_CHECK(esp_wifi_init(&cfg));

esp_event_handler_instance_t instance_any_id;
esp_event_handler_instance_t instance_got_ip;
ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                            ESP_EVENT_ANY_ID,
                            &event_handler,
                            NULL,
                            &instance_any_id));
ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                            IP_EVENT_STA_GOT_IP,
                            &event_handler,
                            NULL,
                            &instance_got_ip));

wifi_config_t wifi_config = {
.sta = {
.ssid = CONFIG_ESP_WIFI_SSID,
.password = CONFIG_ESP_WIFI_PASS,
},
};
ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
ESP_ERROR_CHECK(esp_wifi_start() );

ESP_LOGI(TAG, "wifi_init_sta finished.");

/* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
* number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
pdFALSE,
pdFALSE,
portMAX_DELAY);

/* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
* happened. */
if (bits & WIFI_CONNECTED_BIT) {
ESP_LOGI(TAG, "connected to ap SSID:%s password:%s",
CONFIG_ESP_WIFI_SSID, CONFIG_ESP_WIFI_PASS);
return true;
} else if (bits & WIFI_FAIL_BIT) {
ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s",
CONFIG_ESP_WIFI_SSID, CONFIG_ESP_WIFI_PASS);
} else {
ESP_LOGE(TAG, "UNEXPECTED EVENT");
}
return false;
}

void start_mdns_service()
{
    esp_err_t err = mdns_init();
    if (err) {
        printf("MDNS Init failed: %d\n", err);
        return;
    }
    mdns_hostname_set("ESP32-Rafael");
    mdns_instance_name_set("Rafael's ESP32 Thing");
}


void discover_mdns_hosts(void *pvParameter) {
    while(1){
        ESP_LOGI("mDNS", "Searching for mDNS hosts...");
        
        esp_ip4_addr_t addr;
        esp_err_t err = mdns_query_a("esp32-pasa", 2000, &addr);

        if (err == ESP_OK) {
            ESP_LOGI("mDNS", "Found host: " IPSTR, IP2STR(&addr));
        } else {
            ESP_LOGI("mDNS", "Nohosts found.");
        }

        vTaskDelay(pdMS_TO_TICKS(10000)); 
    }
}

static const char *ip_protocol_str[] = {"V4", "V6", "MAX"};


static void mdns_print_results(mdns_result_t *results)
{
    mdns_result_t *r = results;
    mdns_ip_addr_t *a = NULL;
    int i = 1, t;
    while (r) {
        if (r->esp_netif) {
            printf("%d: Interface: %s, Type: %s, TTL: %" PRIu32 "\n", i++, esp_netif_get_ifkey(r->esp_netif),
                   ip_protocol_str[r->ip_protocol], r->ttl);
        }
        if (r->instance_name) {
            printf("  PTR : %s.%s.%s\n", r->instance_name, r->service_type, r->proto);
        }
        if (r->hostname) {
            printf("  SRV : %s.local:%u\n", r->hostname, r->port);
        }
        if (r->txt_count) {
            printf("  TXT : [%zu] ", r->txt_count);
            for (t = 0; t < r->txt_count; t++) {
                printf("%s=%s(%d); ", r->txt[t].key, r->txt[t].value ? r->txt[t].value : "NULL", r->txt_value_len[t]);
            }
            printf("\n");
        }
        a = r->addr;
        while (a) {
            if (a->addr.type == ESP_IPADDR_TYPE_V6) {
                printf("  AAAA: " IPV6STR "\n", IPV62STR(a->addr.u_addr.ip6));
            } else {
                printf("  A   : " IPSTR "\n", IP2STR(&(a->addr.u_addr.ip4)));
            }
            a = a->next;
        }
        r = r->next;
    }
}
void find_mdns_service(const char * service_name, const char * proto)
{
    ESP_LOGI(TAG, "Query PTR: %s.%s.local", service_name, proto);

    mdns_result_t * results = NULL;
    esp_err_t err = mdns_query_ptr(service_name, proto, 2000, 20,  &results);
    if(err){
        ESP_LOGE(TAG, "Query Failed");
        return;
    }
    if(!results){
        ESP_LOGW(TAG, "No results found!");
        return;
    }

    mdns_print_results(results);
    mdns_query_results_free(results);
}

void find_services(void *pvParameter) {
    while (1) {
        find_mdns_service("_http", "_tcp");
    //or file servers
    find_mdns_service("_smb", "_tcp"); //windows sharing
    find_mdns_service("_afpovertcp", "_tcp"); //apple sharing
    find_mdns_service("_nfs", "_tcp"); //NFS server
    find_mdns_service("_ftp", "_tcp"); //FTP server
    //or networked printer
    find_mdns_service("_printer", "_tcp");
    find_mdns_service("_ipp", "_tcp");
    }
}
