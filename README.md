# Panasonic-Air-Ir-ESP-Home

Panaasonic Climat IR компонент для ESP Home.
Сделано под пульт A75C3747.

##### Не реализовано:
- таймер;
- режим управления потоком воздуха (в ESP Home нет состояний кроме VERTICAL/HORIZONTAL/BOTH/OFF).

##### YAML для добавления в конфигурацию ESP Home
```
external_components:
  - source: github://flight4287/panasonic-air-ir-esphome

remote_transmitter:
  pin: D3
  carrier_duty_percent: 50%
  
remote_receiver:
  id: receiver
  pin:
    number: D5
    inverted: True
    mode: INPUT_PULLUP
  tolerance: 55%
  
climate:
  - platform: panasonic
    name: "Living Room AC"
    receiver_id: receiver

```


За основу взят код от компонента Daikin https://github.com/esphome/esphome/tree/dev/esphome/components/daikin

