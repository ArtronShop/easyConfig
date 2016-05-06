# easyConfig library for ESP8266
ไลบารี่สำหรับการจัดการ การตั้งค่าต่างๆ ผ่านหน้าเว็บโดยใช้คำสั่งสำเร็จรูป หน้าเว็บรองรับ Responsive ทำให้สามารถใช้งานได้ง่ายในทุกอุปกรณ์ ทุกขนาดหน้าจอ อีกทั้งยังรองรับการรีเซ็ตการตั้งค่าทั้งผ่านหน้าเว็บ หรือผ่านการกดปุ่มบนอุปกรณ์ มีระบบล็อกอินก่อนเข้าหน้าการตั้งค่า

### ตัวอย่างหน้าคอนฟิกบนอุปกรณ์ต่างๆ
หน้าจอสมาร์ทโฟน

![enter image description here](http://a.lnwpic.com/o3q49n.jpg)


หน้าจอคอมพิวเตอร์

![enter image description here](http://a.lnwpic.com/bs1pkp.jpg)



## การติดตั้ง
 * ดาว์โหลดไฟล์ไลบารี่ที่ https://github.com/ioxhop/easyConfig/releases/download/1.0.0/easyConfig.zip
 * เปิดโปรแกรม Arduino IDE ขึ้นมา จากนั้นกดไปที่ Sketch > Include Library > Add .ZIP Library...
 * เลือกไฟล์ที่ได้ดาว์โหลดไว้
 * เสร็จสิ้นการติดตั้ง

## การใช้งานเบื้องต้น
 * เปิดตัวอย่าง Basic แล้วอัพโหลดเข้า ESP8266
 * ใช้อุปกรณ์ที่สามารถเชื่อมต่อ WiFi ได้ เชื่อมต่อเข้าไปที่ WiFi ชื่อ ESP_easyConfig
 * ใส่ชื่อผู้ใช้เป็น admin และรหัสเป็น password
 * จะพบหน้าจอคอนฟิก ทดลองเปลี่ยนการตั้งค่าต่างๆ แล้วกดปุ่ม Save
 * ESP8266 จะจำค่าที่กรอกไว้ แล้วรีเซ็ตตัวเอง รอประมาณ 5 - 30 วินาที ถึงจะกลับมาเข้าหน้าคอนฟิกได้อีกครั้ง
 * หากกรอกให้เชื่อมต่อกับ AP ไว้ เมื่อเชื่อมต่อได้สำเร็จ หลอดไฟแสดงสถานะ LED บนตัวโมดูล ESP8266 จะติดสว่างค้าง ขณะกำลังเชื่อมต่อจะกระพริบทุกๆ 100mS

## รายละเอียดฟังก์ชั่น
### void easyConfig::setValue(String key, String val)
ใช้สำหรับแก้ค่าดีฟอลต่างๆ

**ค่าพารามิเตอร์**

*String key* - ชื่อคีย์ของค่าดีฟอลที่ต้อการแก้ ซึ่งเป็นไปได้ดังนี้
 * ssid - แก้ค่าดีฟอลชื่อผู้ใช้ของ AP ที่ต้องการให้โมดูลไปเชื่อมต่อด้วย ปกติแล้วจะมีค่าเป็น ssid[0] = 0
 * password - แก้ค่าดีฟอลรหัสผ่านของ AP ที่ต้องการให้โมดูลไปเชื่อมต่อด้วย ปกติแล้วจะมีค่าเป็น password[0] = 0
 * name - แก้ค่าดีฟอลของชื่ออุปกรณ์ ซึ่งจะถูกนำอ้างอิงเป็น ssid ในโหมด AP ปกติแล้วจะมีค่าเป็น ESP_easyConfig
 * auth-username - แก้ค่าดีฟอลของชื่อผู้ใช้ที่ใช้ล็อกอินเข้าหน้าคอนฟิกบนหน้าเว็บเพจ ปกติแล้วจะมีค่าเป็น admin
 * auth-username - แก้ค่าดีฟอลของรหัสผ่านที่ใช้ล็อกอินเข้าหน้าคอนฟิกบนหน้าเว็บเพจ ปกติแล้วจะมีค่าเป็น password
*String val* - ค่าที่ต้องการจะแก้ สูงสุด 20 ตัวอักษร

### void easyConfig::setMode(WiFiMode mode)
ใช้เซ็ตค่าของโหมดการเชื่อมต่อ


**ค่าพารามิเตอร์**

*WiFiMode mode* - มีค่าที่เป็นได้ได้ดังนี้
 * WIFI_AP - ใช้โหมดเป็น AP ให้อุปกรณ์อื่นเข้ามาเชื่อมต่อ
 * WIFI_STA - ใช้โหมด STA ไปเชื่อมต่อกับอุปกรณ์อื่น (ไม่แนะนำ)
 * WIFI_AP_STA  - ใช้โหมด AP และ STA พร้อมกัน สามารถเชื่อมต่ออุปกรณ์อื่น หรือให้อุปกรณ์อื่นเข้ามาเชื่อมต่อ (เป็นค่าดีฟอล)

### bool easyConfig::isConnect()
ใช้เช็คว่าตอนนี้ได้เชื่อมต่อกับ AP อยู่หรือไม่


**การตอบกลับ**

ให้ค่า true หากเชื่อมต่ออยู่ และให้ค่า false หากไม่ได้เชื่อมต่ออยู่

### void easyConfig::restore(bool reboot)
ใช้ล้างการตั้งค่าที่ได้ตั้งไว้

**ค่าพารามิเตอร์**

*bool reboot* - หากเป็น true จะรีเซ็ตตัวเองเมื่อล้างการตั้งค่าแล้ว (แนะนำ)

### void easyConfig::restoreButton(int pin[, bool activeHigh=true])
ใช้กำหนดขาที่เชื่อมต่อกับปุ่มล้างการตั้งค่า


**ค่าพารามิเตอร์**

*int pin* - กำหนดขา GPIO ที่เชื่อมต่อกับปุ่มอยู่
*bool activeHigh* - (ไม่จำเป็น) ใช้กำหนดว่าปุ่มต่อวงจรแบบ pullUp หรือ pullDown


**ข้อกำหนด**

เมื่อใช้คำสั่งนี้กำหนดปุ่มแล้ว การล้างการตั้งค่าจะต้องกดปุ่มค้างไว้ 5 วินาที จึงจะล้างการตั้งค่าให้ และทำงานแบบ Multitasking คือไม่มีการหยุดโปรแกรมเพื่อรอปล่อยปุ่ม

### void easyConfig::begin(bool runWebServer)
ใช้เริ่มใช้งานไลบารี่ easyConfig


**ค่าพารามิเตอร์**

*bool runWebServer* - กำหนดว่าให้ต้องการรันคำสั่ง server.begin() ด้วยเลยหรือไม่ (แนะนำ true)

### void easyConfig::run()
ฟังก์ชั่นนี้จะต้องอยู่ใน void loop() เพื่อให้ web service สามารถทำงานได้

## ลิขสิทธิ์การใช้งาน
ผู้จัดทำอนุญาตให้ใช้งานได้ในเชิงพาณีชย์ แต่ไม่อนุญาตให้คัดลอกผลงานนี้ไปอัพโหลดใหม่ หรือนำไปหารายได้โดยตรง (เช่น การคัดลอกโค้ดทั้งหมดไปขาย)
 * http://www.ioxhop.com/
 * https://github.com/esp8266/Arduino
