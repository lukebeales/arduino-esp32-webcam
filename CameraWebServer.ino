unsigned long interval = 60;  // in seconds

char web_uuid[64] = {""};     // *  -- if no serial device is connected, this is required!!!

char wifi_ssid[65] = {""};     // 0
char wifi_password[65] = {""}; // 1

char web_ip[16] = {""};     // 2, ip of the server
char web_url[64] = {""};     // 3, this is the path eg. /api/stack
char web_host[64] = {""};      // 4, this is the domain eg. dashboard.resourcible.com

boolean flash = false;      // set by "f" in the bios *** THIS IS BAD IF STREAMING ***
boolean stream = true;      // set by "s" in the bios
