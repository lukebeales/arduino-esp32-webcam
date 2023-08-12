#include "esp_camera.h"
#include "esp_http_client.h"

String serial_buffer = "";

// this needs to capture an image
camera_fb_t * fb = NULL;

#define LED_BUILTIN 4

// this is from the stream.cpp
void startStreamServer();

void serial_buffer_clense() {
  char c;
  for ( int i = 0; i < serial_buffer.length()-1; ++i ) {
        c = serial_buffer.charAt(i);
        if (
          ( c == 0 ) ||
          ( c == 13 ) ||
          ( c == 2 ) ||
          ( c == 3 )
        ) {
            serial_buffer.remove(i, 1);
        }
    }
}

void serial_buffer_empty() {
  while(Serial.available() > 0) {
    char t = Serial.read();
  }
}

void setup() {

  // get the onboard led ready
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  // set the serial port
  Serial.begin(9600);
  
  // Configuring GPIO...
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = 5;
  config.pin_d1 = 18;
  config.pin_d2 = 19;
  config.pin_d3 = 21;
  config.pin_d4 = 36;
  config.pin_d5 = 39;
  config.pin_d6 = 34;
  config.pin_d7 = 35;
  config.pin_xclk = 0;
  config.pin_pclk = 22;
  config.pin_vsync = 25;
  config.pin_href = 23;
  config.pin_sscb_sda = 26;
  config.pin_sscb_scl = 27;
  config.pin_pwdn = 32;
  config.pin_reset = -1;

  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  //init with high specs to pre-allocate larger buffers
  config.frame_size = FRAMESIZE_UXGA;
  config.jpeg_quality = 10; // 12
  config.fb_count = 1;      // 2


  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    // Camera init failed with error 0x%x
    return;
  }

}


/////////////////////////////////////////////////////////////////////////////


void loop() {

  unsigned long loop_timestamp = millis();
  boolean serial_enabled = false;

  // if no uuid was specified manually, then lets look to the serial port...
  if ( strlen(web_uuid) < 10 ) {

    // see if the serial is good...
    serial_buffer_empty();
    Serial.println("BIOS+GET=*");
    serial_buffer = Serial.readStringUntil('\n');
    serial_buffer_clense();
    if ( serial_buffer.length() >= 10 ) {
  
      serial_enabled = true;
      serial_buffer.toCharArray(web_uuid, serial_buffer.length());
      web_uuid[serial_buffer.length()] = '\0';
    }
  
    if ( serial_enabled ) {
  
      // wait a bit
      delay(250);
  
      serial_buffer_empty();
      Serial.println("BIOS+GET=0");
      serial_buffer = Serial.readStringUntil('\n');
      serial_buffer_clense();
      serial_buffer.toCharArray(wifi_ssid, serial_buffer.length());
      wifi_ssid[serial_buffer.length()] = '\0';
  
      // wait a bit
      delay(250);
    
      serial_buffer_empty();
      Serial.println("BIOS+GET=1");
      serial_buffer = Serial.readStringUntil('\n');
      serial_buffer_clense();
      serial_buffer.toCharArray(wifi_password, serial_buffer.length());
      wifi_password[serial_buffer.length()] = '\0';
  
      // wait a bit
      delay(250);
  
  
      serial_buffer_empty();
      Serial.println("BIOS+GET=2");
      serial_buffer = Serial.readStringUntil('\n');
      serial_buffer_clense();
      serial_buffer.toCharArray(web_ip, serial_buffer.length());
      web_ip[serial_buffer.length()] = '\0';
  
      // wait a bit
      delay(250);
  
      serial_buffer_empty();
      Serial.println("BIOS+GET=3");
      serial_buffer = Serial.readStringUntil('\n');
      serial_buffer_clense();
      serial_buffer.toCharArray(web_url, serial_buffer.length());
      web_url[serial_buffer.length()] = '\0';
  
      // wait a bit
      delay(250);
  
      serial_buffer_empty();
      Serial.println("BIOS+GET=4");
      serial_buffer = Serial.readStringUntil('\n');
      serial_buffer_clense();
      serial_buffer.toCharArray(web_host, serial_buffer.length());
      web_host[serial_buffer.length()] = '\0';
  
      // wait a bit
      delay(250);
  
      // this is the flash led
      serial_buffer_empty();
      Serial.println("BIOS+GET=f");
      serial_buffer = Serial.readStringUntil('\n');
      serial_buffer_clense();
      if (
        ( serial_buffer[0] == '1' ) ||
        ( serial_buffer[0] == 'y' )
      ) {
        flash = true;
      }
  
      // wait a bit
      delay(250);
  
      // this is the streaming server
      serial_buffer_empty();
      Serial.println("BIOS+GET=s");
      serial_buffer = Serial.readStringUntil('\n');
      serial_buffer_clense();
      if (
        ( serial_buffer[0] == '1' ) ||
        ( serial_buffer[0] == 'y' )
      ) {
        stream = true;
      }
  
    }
  }
  
  

  ////////////////////////////////////////////////


  if ( wifi_open() ) {

    if ( stream ) {
      if ( flash == true ) {
        digitalWrite(LED_BUILTIN, HIGH);
      }

      // start the stream server if enabled
      startStreamServer();
    }

    while (true) {

      // this is so we don't keep uploading images every 3 seconds...
      loop_timestamp = millis();

      /* this is the esp32 way
        // https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/protocols/esp_http_client.html#_CPPv424esp_http_client_config_t
        esp_http_client_config_t config = {0};
        config.host = web_ip;
        config.path = web_url;
        config.port = 80;
        config.user_agent = "ESP32cam uploader";
        esp_http_client_handle_t client = esp_http_client_init(&config);
        esp_http_client_set_method(client, HTTP_METHOD_POST);
      
        // esp_http_client_set_post_field(client, post_data, strlen(post_data));
      
        esp_http_client_set_header(client, "Host", web_host);
        esp_http_client_set_header(client, "X-UUID", web_uuid);
        esp_http_client_set_header(client, "Content-Type", "multipart/form-data; boundary=AaB03x");
        esp_http_client_set_header(client, "Content-Length", (const char *)&full_length);
  
  
        esp_http_client_open(client, full_length);
        
        char startBuf[start_request.length()+1];
        start_request.toCharArray(startBuf, start_request.length()+1);
        esp_http_client_write(client, startBuf, start_request.length());
  
          // chunking method found here: https://randomnerdtutorials.com/esp32-cam-post-image-photo-server/
            uint8_t *fbBuf = fb->buf;
            size_t fbLen = fb->len;
            for (size_t n=0; n<fbLen; n=n+1024) {
              if (n+1024 < fbLen) {
                // client.write(fbBuf, 1024);
                esp_http_client_write(client, (const char *)fbBuf, 1024);
                fbBuf += 1024;
              } else if (fbLen%1024>0) {
                size_t remainder = fbLen%1024;
                // client.write(fbBuf, remainder);
                esp_http_client_write(client, (const char *)fbBuf, remainder);
              }
            } 
          //  
  
        char endBuf[end_request.length()+1];
        end_request.toCharArray(endBuf, end_request.length()+1);
        esp_http_client_write(client, endBuf, end_request.length());
  
        delay(1000);
        esp_http_client_close(client);
        esp_http_client_cleanup(client);
      */

      WiFiClient client;
  
  
      
      // client.write seems to run in a different order to client.print - on the first loop only!  This causes broken requests.
      // https://esp8266-arduino-spanish.readthedocs.io/es/latest/esp8266wifi/client-class.html
      // https://forum.arduino.cc/t/esp8266-how-to-force-wificlient-to-send-so-far-written-bytes-to-the-network/555942/20
      client.setNoDelay(true);     
      // no fix for esp32 :(
      // https://github.com/espressif/arduino-esp32/issues/2576
      // client.setSync(true);     
  
  
      if ( client.connect(web_ip, 80) ) {
  
        //
        
          sensor_t * s = esp_camera_sensor_get();
        
          //initial sensors are flipped vertically and colors are a bit saturated
          if (s->id.PID == OV3660_PID) {
            s->set_vflip(s, 1);//flip it back
            s->set_brightness(s, 1);//up the blightness just a bit
            s->set_saturation(s, -2);//lower the saturation
          }
        
          s->set_whitebal(s, 1);       // 0 = disable , 1 = enable
          s->set_awb_gain(s, 1);       // 0 = disable , 1 = enable
          s->set_wb_mode(s, 0);        // 0 to 4 - if awb_gain enabled (0 - Auto, 1 - Sunny, 2 - Cloudy, 3 - Office, 4 - Home)
          s->set_exposure_ctrl(s, 1);  // 0 = disable , 1 = enable
        
          // delay here to give the power a chance to settle
          delay(500);
      
        //
  
 
        if ( stream ) {
          // just set it at the smallest framesize.  solves a lot of issues
          s->set_framesize(s, FRAMESIZE_QVGA);  // 320 x 200
        } else {
          byte $framesize = 4; // start at SXGA
          // start with the largest size
          s->set_framesize(s, FRAMESIZE_SXGA);  // 1280 x 1024

          if ( flash == true ) {
            digitalWrite(LED_BUILTIN, HIGH);
          }
    
          // stream for a while so that the exposure can come back to normal levels.
          // this resolves the 'red' tint eg. https://github.com/espressif/esp32-camera/issues/314
            unsigned long exposure_timestamp = millis();
            while ( ((6) - round((millis() - exposure_timestamp) / 1000)) > 0 ) {
    
              for ( byte i = 0; i < 10; i++ ) {
                fb = esp_camera_fb_get();
                esp_camera_fb_return(fb);
                delay(50);
              }
              
              fb = esp_camera_fb_get();
              // if the image is too large then shift down to the next size.
              if ( ( fb->len > 60000 ) && ( $framesize > 1 ) ) {
                $framesize = $framesize - 1;
    
                // set the framesize to capture
                // i know this is a terrible way of doing it
                if ( $framesize == 3 ) {
                  s->set_framesize(s, FRAMESIZE_SVGA);  // 800 x 600
                } else if ( $framesize == 2 ) {
                  s->set_framesize(s, FRAMESIZE_VGA);   // 640 x 480
                } else {
                  s->set_framesize(s, FRAMESIZE_QVGA);  // 320 x 200
                }
                delay(500);
              }
              esp_camera_fb_return(fb);
              delay(50);
            }
          //
        }

        // now get the real image
        fb = esp_camera_fb_get();

        if ( ! stream ) {
          digitalWrite(LED_BUILTIN, LOW);
        }
                
        if (!fb) {
          // Camera capture failed.
        } else if ( fb->len > 60000 ) {
          // image size too large, forgeddaboudit
        } else {
          // Camera capture succeeded!
      
         
          ///////
      
            // figure out the length of the content
            uint16_t len = fb->len;
      
            String start_request = "";
            String end_request = "";
            start_request = start_request +
              "--AaB03x\r\n" + 
              "Content-Disposition: form-data; name=\"image\"; filename=\"image.jpg\"\r\n" + 
              "Content-Type: image/jpeg\r\n" +
              "Content-Transfer-Encoding: binary\r\n\r\n";
      
            end_request = end_request + "\r\n" +
              "--AaB03x--\r\n";
      
            uint16_t full_length = start_request.length() + len + end_request.length();
        
            String web_local_ip = wifi_ip();
            
            String header_request = "POST " + String(web_url) + " HTTP/1.1\r\n" +
              "Host: " + String(web_host) + "\r\n" +
              "User-Agent: ESP32cam uploader\r\n" +
              "X-UUID: " + String(web_uuid) + "\r\n" +
              "X-IP: " + web_local_ip + "\r\n" +
              "Content-Type: multipart/form-data; boundary=AaB03x" + "\r\n" +
              "Content-Length: " + String(full_length) + "\r\n" + 
              "\r\n";
  
          ///////////        
          
          // this is done in one go because otherwise it breaks packets somehow.
          // also google cloud platform (GCP) rearranges packets.  Thanks google!        
          client.print(header_request);
          client.print(start_request);
  
          // lets just transfer the whole thing
          client.write((const char*)fb->buf, len);
  
          // hand the camera back.
          esp_camera_fb_return(fb);
  /*
            // chunking method found here: https://randomnerdtutorials.com/esp32-cam-post-image-photo-server/
              uint8_t *fbBuf = fb->buf;
              size_t fbLen = fb->len;
              for (size_t n=0; n<fbLen; n=n+1024) {
                if (n+1024 < fbLen) {
                  client.write(fbBuf, 1024);
                  fbBuf += 1024;
                } else if (fbLen%1024>0) {
                  size_t remainder = fbLen%1024;
                  client.write(fbBuf, remainder);
                }
              } 
            //  
  */
          client.print(end_request);
  
        }
  
        // Closing connection...
        delay(2000);    // this needs to allow for slow websites :(
        client.stop();
  
      }
  
      if ( ! stream ) {
        wifi_close();
  
        // if we have serial, send a sleep command as we're finished
        if ( serial_enabled ) {
          Serial.println("BIOS+ZZZ");
        }
      }
  
      // wait for what's left of N seconds
      while ( ((interval) - round((millis() - loop_timestamp) / 1000)) > 0 ) {
        delay(1000);
      }
  
      if ( ! stream ) {
        wifi_open();
      }

    }

  }       

}
