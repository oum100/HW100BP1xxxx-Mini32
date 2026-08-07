# MQTT Protocol Reference

เอกสารนี้เป็น baseline ของ MQTT protocol ที่เฟิร์มแวร์ใช้งานจริง ณ วันที่ 2026-08-07 อ้างอิงจาก `src/main.cpp` และ `src/config.cpp` การแก้ protocol หลังจากนี้ต้องอัปเดตเอกสารนี้พร้อมโค้ด และต้องรักษา backward compatibility หรือประกาศ protocol version ใหม่

## 1. Primary MQTT connection

| Parameter | ค่าที่ใช้งานปัจจุบัน |
|---|---|
| Library | PubSubClient 2.8.0 |
| Default broker | `mq3.payboard.cc` |
| Port | `1883` |
| TLS | ไม่มี |
| Client ID | Device ID เลขฐานสิบหก 12 ตัวจาก ESP32 eFuse MAC |
| Username | Merchant ID ตามค่าเริ่มต้น หรือ `mqttuser` จาก NVS เมื่อมี MQTT config |
| Password | Merchant key ตามค่าเริ่มต้น หรือ `mqttpass` จาก NVS เมื่อมี MQTT config |
| Keepalive | 30 วินาที |
| MQTT buffer | 512 bytes |
| QoS | 0 โดยปริยาย |
| Retained message | ไม่ใช้ |
| Reconnect | สูงสุด 5 ครั้ง เว้นประมาณ 3 วินาที แล้วรีสตาร์ตอุปกรณ์ |

Credential จริงเป็นข้อมูลลับ ห้ามใส่ในเอกสาร log หรือ payload ตัวอย่าง ค่า broker, port, username และ password สามารถถูก override จาก ESP32 Preferences namespace `config`

Secondary Flipup MQTT มีโค้ดรองรับแต่ `FLIPUPMQTT` ไม่ได้เปิดใน build ปัจจุบัน จึงไม่ถือเป็น protocol หลัก

## 2. Topic structure

### Normal operation

| Direction | Topic | หน้าที่ |
|---|---|---|
| Backend → Device | `payboard/<merchantid>/<uuid>` | ส่งคำสั่ง JSON ให้อุปกรณ์ |
| Device → Backend | `payboard/backend/<merchantid>/<uuid>` | ส่ง response และ status |

### Legacy MQTT registration flow

| Direction | Topic | Payload |
|---|---|---|
| Device → Backend | `payboard/register` | `merchantid`, `mac`, `model`, `firmware` |
| Backend → Device | `payboard/<merchantid>/<mac>` | `{"uuid":"..."}` |

เมื่อได้รับ UUID อุปกรณ์จะบันทึกลง NVS และเปลี่ยนไป subscribe topic ที่ลงท้ายด้วย UUID ปัจจุบัน `setup()` ใช้ HTTP registration เป็นหลักเมื่อ UUID ไม่มี ดังนั้น MQTT registration ถือเป็นเส้นทางเดิมที่ยังอยู่ในโค้ด

## 3. Message envelope

### Request ปัจจุบัน

```json
{
  "action": "ping"
}
```

- `action` เป็น string และบังคับใช้สำหรับทุกคำสั่ง
- Parameter อื่นอยู่ระดับเดียวกับ `action`
- ไม่มี `requestId`, timestamp หรือ protocol version ใน protocol ปัจจุบัน
- Payload ถูก parse ด้วย ArduinoJson document ขนาด 1024 bytes แต่ MQTT buffer จำกัด 512 bytes ดังนั้น payload จริงต้องไม่เกินข้อจำกัด MQTT buffer

### Response ปัจจุบัน

```json
{
  "response": "ping",
  "merchantid": "<merchantid>",
  "uuid": "<uuid>",
  "state": "Available",
  "desc": "..."
}
```

- โดยทั่วไป `response` ใช้ชื่อเดียวกับ `action`
- Response ถูก publish กลับทาง Device → Backend topic
- บางคำสั่งรีบูตทันทีหรือมี implementation bug ทำให้ response อาจไม่ถูกส่ง
- การพัฒนารุ่นถัดไปควรเพิ่ม optional `protocolVersion` และ `requestId` โดยไม่ทำให้ client v1 เดิมเสีย

## 4. Device states

| Internal state | MQTT state | ความหมาย |
|---:|---|---|
| 1 | — | ยังไม่ configure/register |
| 2 | — | register แล้ว รอ activate/config |
| 3 | `Available` | พร้อมรับบริการ |
| 4 | `Booked` | รับเงินแล้วบางส่วนหรือกำลังเลือก package |
| 5 | `Busy` | เครื่องกำลังทำงาน |
| 6 | — | เริ่มงานไม่สำเร็จ |
| 10 | `Offline` | ปิดรับบริการ |

## 5. Operational commands

### `ping`

ขอสถานะอุปกรณ์

```json
{"action":"ping"}
```

Response เพิ่ม `rssi`, `firmware`, `timeRemain` และ `state`

### `paid`

แจ้งว่า QR payment สำเร็จ ค่า `price` ต้องตรงกับ product ที่ configure ไว้

```json
{
  "action": "paid",
  "price": 30,
  "orderNo": "ORDER001"
}
```

อุปกรณ์กำหนด `paymentby=2`, เก็บ `orderNo` และเริ่มงานผ่าน state machine

### `jobcreate`

สร้างงานจาก Kiosk/Admin

```json
{
  "action": "jobcreate",
  "price": 30,
  "paymentby": 4
}
```

`paymentby`: 1=coin, 2=QR, 3=kiosk, 4=free/admin

### Direct machine controls

คำสั่งไม่มี parameter เพิ่ม:

```json
{"action":"spin"}
{"action":"rinsespin"}
{"action":"selfclean"}
{"action":"startpause"}
{"action":"turnon"}
{"action":"turnoff"}
{"action":"jobcancel"}
{"action":"resetstate"}
{"action":"selftest"}
```

ข้อจำกัดสำคัญ:

- `jobcancel` ยอมรับเฉพาะช่วงต้นของงานตามเวลาที่โค้ดตรวจ
- `resetstate` หยุด timer และล้างสถานะงาน
- Direct controls กระทบ relay และเครื่องจริง ต้องจำกัดสิทธิ์ฝั่ง publisher
- `selftest` ทำงานจริง แต่ response ในโค้ดปัจจุบันใส่ชื่อและ state ผิดเป็น `nvsdelete`

### Availability controls

```json
{"action":"offline"}
{"action":"online"}
```

`offline` ปิด coin acceptor และบันทึก state 10; `online` ล้าง state กลับสู่พร้อมบริการ

## 6. Product and service configuration

### `config`

ตั้งค่าได้สูงสุด 3 products

```json
{
  "action": "config",
  "detail": [
    {"sku":"P1","price":30,"period":35,"unit":2},
    {"sku":"P2","price":40,"period":67,"unit":2},
    {"sku":"P3","price":50,"period":97,"unit":2}
  ]
}
```

| Field | Type | ความหมาย |
|---|---|---|
| `sku` | string | รหัสสินค้า |
| `price` | number | ราคา ต้องตรงกับจำนวนที่ใช้เลือกโปรแกรม |
| `period` | integer | ระยะเวลาบริการ |
| `unit` | integer | 0=none, 1=second, 2=minute, 3=milliliter, 4=liter |

ถ้า `period=0` เฟิร์มแวร์จะคำนวณเวลาจากราคา Product ที่เกินจำนวนรายการใหม่จะถูกล้างออกจาก NVS

### `stateupdate`

```json
{"action":"stateupdate","available":60,"busy":1}
```

บันทึกช่วงเวลารายงานสถานะสำหรับ Available และ Busy เป็นนาที แต่ implementation ปัจจุบันตั้ง periodic status คงที่ 60 นาที และ Busy จะส่งเพิ่มทุก 1 นาทีผ่าน `serviceLeft()` ดังนั้น parameter นี้ยังไม่ได้ควบคุม timer ทั้งหมดจริง

### `coinmodule`

```json
{"action":"coinmodule","coinmodule":"single"}
{"action":"coinmodule","coinmodule":"multi"}
```

- `single`: 1 pulse = 10 หน่วยเงิน
- ค่าอื่นรวมถึง `multi`: 1 pulse = 1 หน่วยเงิน
- ปัจจุบันเปลี่ยนค่าใน RAM แต่ไม่ได้ persist ลง NVS ใน branch นี้

### `coinwaittimeout`

```json
{"action":"coinwaittimeout","coinwaittimeout":0.5}
```

หน่วยเป็นนาที ใช้หน่วงก่อนเริ่มงานเพื่อเปิดโอกาสให้เพิ่มเงินเลือก package ที่สูงกว่า และบันทึกลง NVS

### `assettype`

```json
{"action":"assettype","assettype":0}
```

ค่าตามโครงสร้างคือ 0=washer, 1=dryer แต่โค้ดปัจจุบันเขียนค่า 0 ลง NVS เสมอ ไม่ว่ารับค่าใด

## 7. Network and identity configuration

### `setwifi`

```json
{
  "action": "setwifi",
  "index": 1,
  "ssid": "<ssid>",
  "key": "<password>",
  "reconnect": 1
}
```

- `index` ใช้เลือก slot ของ Wi-Fi config
- `reconnect=1` ให้ตัดและเชื่อมต่อใหม่ทันที
- ห้ามบันทึก payload จริงที่มี `key` ลง log หรือเอกสาร

### `setmerchant`

```json
{
  "action": "setmerchant",
  "merchantid": "<merchantid>",
  "merchantkey": "<secret>"
}
```

บันทึกแล้วรีบูต ปัจจุบัน startup โหลด `merchantid` แต่ไม่ได้โหลด `merchantkey` จาก NVS ในเส้นทางเดียวกัน จึงต้องแก้ก่อนยึดคำสั่งนี้เป็น provisioning ที่เชื่อถือได้

### `setmqtt`

```json
{
  "action": "setmqtt",
  "mqtthost": "broker.example",
  "mqttport": 1883,
  "mqttuser": "<user>",
  "mqttpass": "<secret>"
}
```

บันทึกแล้วรีบูต ปัจจุบัน branch นี้เขียน `mqttport` เป็น string แต่ startup อ่านเป็น integer และรีบูตก่อน publish response จึงต้องแก้ type และ acknowledgement flow

### `setmac` / `delmac`

```json
{"action":"setmac","mac":"AA:BB:CC:DD:EE:FF"}
{"action":"delmac"}
```

เปลี่ยน/ลบ fixed MAC และ UUID แล้วรีบูต ส่งผลต่อ identity และ topic ของอุปกรณ์

### `setntp`

```json
{"action":"setntp","ntpinx":1,"value":"asia.pool.ntp.org"}
```

โค้ดอ่าน server จาก field `value` ไม่ใช่ `ntpserver` ตาม comment เดิม และยังไม่ได้ persist ค่าลง NVS

### `payboard`

ใช้แก้ Payboard parameters แบบรายค่า

```json
{"action":"payboard","params":"uuid","uuid":"<uuid>"}
{"action":"payboard","params":"merchantid","merchantid":"<merchantid>"}
{"action":"payboard","params":"merchantkey","merchantkey":"<secret>"}
{"action":"payboard","params":"apihost","apihost":"https://..."}
{"action":"payboard","params":"apikey","apikey":"<secret>"}
```

สำหรับ `params="mqtthost"` หรือ `params="all"` โค้ดปัจจุบันอ่าน port จาก field ที่สะกดผิดว่า `mqttportt` ห้ามนำ typo นี้ไปเป็นมาตรฐานถาวร ควรแก้โค้ดให้รองรับ `mqttport` และรองรับชื่อเดิมชั่วคราวเพื่อ compatibility

## 8. Firmware and maintenance commands

```json
{"action":"firmware"}
{"action":"ota"}
{"action":"reset"}
{"action":"reboot"}
{"action":"restart"}
{"action":"nvsdelete"}
```

- `firmware`: อ่านเวอร์ชันปัจจุบัน
- `ota`: ตรวจ manifest, ดาวน์โหลด firmware ผ่าน HTTPS และรีบูต
- `reset`, `reboot`, `restart`: เป็น alias สำหรับรีบูต
- `nvsdelete`: ลบ NVS ทั้งหมดและรีบูต เป็นคำสั่งทำลาย configuration

คำสั่งเหล่านี้ต้องส่งจากระบบที่ยืนยันสิทธิ์แล้วเท่านั้น `nvsdelete` รีบูตก่อน generic response publish จึงอาจไม่มี acknowledgement ก่อน offline

## 9. Automatic status messages

อุปกรณ์ส่ง status เมื่อเริ่มงาน ทุกหนึ่งนาทีระหว่างทำงาน และ periodic timer 60 นาที

```json
{
  "response": "Status",
  "time": "<ctime string>",
  "event": "<event name>",
  "merchantid": "<merchantid>",
  "uuid": "<uuid>",
  "rssi": -55,
  "state": "Busy",
  "firmware": "1.0.12",
  "timeRemain": 34
}
```

`timeRemain` ใช้หน่วยนาทีใน flow ปัจจุบัน

## 10. Present but incomplete actions

| Action | สถานะปัจจุบัน |
|---|---|
| `countcoin` | โค้ดถูก comment ไม่มีการทำงาน |
| `orderid` | มี branch แต่ไม่มี logic |
| `backend` | มี branch แต่ไม่มี logic |
| `stateflag` | มี branch แต่ไม่มี logic |

ห้ามพึ่งพาคำสั่งเหล่านี้ใน backend ใหม่จนกว่าจะกำหนด contract และ implement พร้อม test

## 11. Compatibility rules for future changes

1. ห้ามเปลี่ยนชื่อ normal-operation topics โดยไม่มี migration plan
2. Request ต้องมี `action`; response ต้องมี `response`, `merchantid`, `uuid`, `state` และ `desc` เมื่อเกี่ยวข้อง
3. จำนวนเงินต้องเป็น number และต้องตรงกับ product ที่ configure; unknown price ต้องถูก reject อย่างชัดเจน
4. คำสั่งหนึ่งต้องมี acknowledgement เดียวที่สัมพันธ์กับ request; ควรเพิ่ม `requestId`
5. ห้ามส่ง credential กลับใน response หรือเขียน credential ลง Serial log
6. คำสั่งทำลายข้อมูล เปลี่ยน identity, OTA และ direct machine control ต้องมี authorization และ audit log ฝั่ง backend
7. เพิ่ม field ใหม่ได้แบบ optional; การลบหรือเปลี่ยนความหมาย field ต้องเพิ่ม protocol version
8. แก้ typo ปัจจุบันด้วยการรองรับทั้ง field เก่าและใหม่ในช่วงเปลี่ยนผ่าน ไม่สร้าง client ใหม่ให้ใช้ typo
9. เพิ่ม test อย่างน้อยสำหรับ valid command, malformed JSON, missing field, unknown action, duplicate delivery, reconnect และ payload เกินขนาด
