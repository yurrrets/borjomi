EESchema Schematic File Version 4
LIBS:Borjomi-cache
EELAYER 29 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title "My Scheme"
Date ""
Rev ""
Comp "Home"
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L MCU_Module:Arduino_Nano_v3.x A1
U 1 1 5B6D71E4
P 4550 2750
F 0 "A1" H 4850 1750 50  0000 C CNN
F 1 "Arduino_Nano" H 5050 1650 50  0000 C CNN
F 2 "Module:Arduino_Nano" H 4700 1800 50  0001 L CNN
F 3 "http://www.mouser.com/pdfdocs/Gravitech_Arduino_Nano3_0.pdf" H 4550 1750 50  0001 C CNN
	1    4550 2750
	1    0    0    -1  
$EndComp
$Comp
L CAN:MCP2515-TJA1050 U1
U 1 1 5B6F5288
P 7600 2350
F 0 "U1" H 7600 3128 50  0000 C CNN
F 1 "MCP2515-TJA1050" H 7600 3037 50  0000 C CNN
F 2 "CAN:AOC346_MCP2515" H 7600 1450 50  0001 C CIN
F 3 "http://arduino.ua/prod1347-modyl-can-shini-na-mcp2515-i-tja1050" H 7700 1550 50  0001 C CNN
	1    7600 2350
	1    0    0    -1  
$EndComp
Wire Wire Line
	3850 1450 3850 2350
Wire Wire Line
	3850 2350 4050 2350
Wire Wire Line
	7000 2200 5800 2200
Wire Wire Line
	5800 2200 5800 4250
Wire Wire Line
	5800 4250 3900 4250
Wire Wire Line
	3900 4250 3900 3450
Wire Wire Line
	3900 3450 4050 3450
Wire Wire Line
	7000 2300 5950 2300
Wire Wire Line
	5950 4350 3800 4350
Wire Wire Line
	3800 4350 3800 3250
Wire Wire Line
	3800 3250 4050 3250
Wire Wire Line
	5950 2300 5950 4350
Wire Wire Line
	7000 2400 6100 2400
Wire Wire Line
	6100 2400 6100 4450
Wire Wire Line
	6100 4450 3700 4450
Wire Wire Line
	3700 3350 4050 3350
Wire Wire Line
	3700 3350 3700 4450
Wire Wire Line
	7000 2500 6250 2500
Wire Wire Line
	6250 2500 6250 4550
Wire Wire Line
	6250 4550 3600 4550
Wire Wire Line
	3600 4550 3600 3050
Wire Wire Line
	3600 3050 4050 3050
Wire Wire Line
	7000 2100 5800 2100
Wire Wire Line
	5800 2100 5800 1450
Wire Wire Line
	5800 1450 3850 1450
Wire Wire Line
	4750 1750 6200 1750
$Comp
L Connector:Screw_Terminal_01x02 J1
U 1 1 5B6F66B3
P 1550 1850
F 0 "J1" H 1470 1525 50  0000 C CNN
F 1 "Vin 12V" H 1470 1616 50  0000 C CNN
F 2 "Connector_Wire:SolderWirePad_1x02_P5.08mm_Drill1.5mm" H 1550 1850 50  0001 C CNN
F 3 "~" H 1550 1850 50  0001 C CNN
	1    1550 1850
	-1   0    0    1   
$EndComp
$Comp
L Connector:Screw_Terminal_01x02 J2
U 1 1 5B6F678B
P 1550 2300
F 0 "J2" H 1470 1975 50  0000 C CNN
F 1 "Vout 12V" H 1470 2066 50  0000 C CNN
F 2 "Connector_Wire:SolderWirePad_1x02_P5.08mm_Drill1.5mm" H 1550 2300 50  0001 C CNN
F 3 "~" H 1550 2300 50  0001 C CNN
	1    1550 2300
	-1   0    0    1   
$EndComp
$Comp
L power:GND #PWR04
U 1 1 5B6F6C10
P 4550 3900
F 0 "#PWR04" H 4550 3650 50  0001 C CNN
F 1 "GND" H 4555 3727 50  0000 C CNN
F 2 "" H 4550 3900 50  0001 C CNN
F 3 "" H 4550 3900 50  0001 C CNN
	1    4550 3900
	1    0    0    -1  
$EndComp
Wire Wire Line
	4550 3750 4550 3850
$Comp
L power:GND #PWR01
U 1 1 5B6F704F
P 2250 2350
F 0 "#PWR01" H 2250 2100 50  0001 C CNN
F 1 "GND" H 2255 2177 50  0000 C CNN
F 2 "" H 2250 2350 50  0001 C CNN
F 3 "" H 2250 2350 50  0001 C CNN
	1    2250 2350
	1    0    0    -1  
$EndComp
Wire Wire Line
	1750 1850 2250 1850
Wire Wire Line
	2250 1850 2250 2300
Wire Wire Line
	1750 2300 2250 2300
Connection ~ 2250 2300
Wire Wire Line
	2250 2300 2250 2350
Wire Wire Line
	1750 2200 2100 2200
Wire Wire Line
	2100 2200 2100 1750
Wire Wire Line
	2100 1750 1750 1750
$Comp
L Connector:RJ14 J4
U 1 1 5B6F8A1D
P 1350 4200
F 0 "J4" H 1100 3850 50  0000 C CNN
F 1 "Data in" H 1500 3850 50  0000 C CNN
F 2 "Connector:RJ14_YH-57-21_5722_6P4C" H 1350 4200 50  0001 C CNN
F 3 "https://www.rcscomponents.kiev.ua/product/rozetka-yh-57-21-5722-6p4c_12940.html" H 1350 4200 50  0001 C CNN
	1    1350 4200
	1    0    0    -1  
$EndComp
$Comp
L Connector:RJ14 J5
U 1 1 5B6F8AB0
P 1350 3350
F 0 "J5" H 1100 3000 50  0000 C CNN
F 1 "Data out" H 1500 3000 50  0000 C CNN
F 2 "Connector:RJ14_YH-57-21_5722_6P4C" H 1350 3650 50  0001 C CNN
F 3 "https://www.rcscomponents.kiev.ua/product/rozetka-yh-57-21-5722-6p4c_12940.html" H 1350 3350 50  0001 C CNN
	1    1350 3350
	1    0    0    -1  
$EndComp
Wire Wire Line
	1750 4300 1900 4300
Wire Wire Line
	1900 4300 1900 3450
Wire Wire Line
	1900 3450 1750 3450
Wire Wire Line
	1750 4200 1950 4200
Wire Wire Line
	1950 3350 1750 3350
Wire Wire Line
	2000 4100 2000 3250
Wire Wire Line
	2050 4000 2050 3150
Wire Wire Line
	1950 4200 1950 4750
Wire Wire Line
	8200 4750 8200 2600
Connection ~ 1950 4200
Wire Wire Line
	8200 2500 8300 2500
Wire Wire Line
	8300 2500 8300 4800
Wire Wire Line
	1900 4800 1900 4300
Connection ~ 1900 4300
Wire Wire Line
	2100 1750 2400 1750
Connection ~ 2100 1750
Wire Wire Line
	1950 4750 8200 4750
Wire Wire Line
	1900 4800 8300 4800
$Comp
L power:GND #PWR03
U 1 1 5B7075AE
P 3500 3350
F 0 "#PWR03" H 3500 3100 50  0001 C CNN
F 1 "GND" H 3505 3177 50  0000 C CNN
F 2 "" H 3500 3350 50  0001 C CNN
F 3 "" H 3500 3350 50  0001 C CNN
	1    3500 3350
	1    0    0    -1  
$EndComp
Wire Wire Line
	3300 2700 3300 2450
Wire Wire Line
	3300 2450 4050 2450
$Comp
L Connector:Screw_Terminal_01x02 J3
U 1 1 5B709E73
P 2600 1250
F 0 "J3" V 2600 1050 50  0000 C CNN
F 1 "Water Switch 12V" V 2450 750 50  0000 C CNN
F 2 "Connector_Wire:SolderWirePad_1x02_P5.08mm_Drill1.5mm" H 2600 1250 50  0001 C CNN
F 3 "~" H 2600 1250 50  0001 C CNN
	1    2600 1250
	0    -1   -1   0   
$EndComp
$Comp
L power:GND #PWR02
U 1 1 5B70B3E3
P 2700 1550
F 0 "#PWR02" H 2700 1300 50  0001 C CNN
F 1 "GND" H 2705 1377 50  0000 C CNN
F 2 "" H 2700 1550 50  0001 C CNN
F 3 "" H 2700 1550 50  0001 C CNN
	1    2700 1550
	1    0    0    -1  
$EndComp
Wire Wire Line
	2700 1450 2700 1550
Wire Wire Line
	2600 1450 2600 2700
Wire Wire Line
	2400 1750 2400 2800
Connection ~ 2400 1750
Wire Wire Line
	2400 1750 4450 1750
$Comp
L Borjomi-rescue:MH-Series-Sensors U2
U 1 1 5B713FC4
P 6600 1150
F 0 "U2" H 6600 1525 50  0000 C CNN
F 1 "MH-Series" H 6600 1434 50  0000 C CNN
F 2 "MH_Sensor:MH_Sensor" H 6600 750 50  0001 C CNN
F 3 "https://ru.aliexpress.com/item/1pcs-lot-soil-the-hygrometer-detection-module-soil-moisture-sensor-Robot-smart-car/32826425650.html?spm=a2g0v.10010108.1000016.1.7ca9471dkqoaPU&isOrigTitle=true" H 6500 1150 50  0001 C CNN
	1    6600 1150
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR05
U 1 1 5B7140ED
P 6050 1400
F 0 "#PWR05" H 6050 1150 50  0001 C CNN
F 1 "GND" H 6055 1227 50  0000 C CNN
F 2 "" H 6050 1400 50  0001 C CNN
F 3 "" H 6050 1400 50  0001 C CNN
	1    6050 1400
	1    0    0    -1  
$EndComp
Wire Wire Line
	6200 1200 6050 1200
Wire Wire Line
	6050 1200 6050 1400
Wire Wire Line
	6200 1300 6200 1750
Connection ~ 6200 1750
Wire Wire Line
	6200 1750 7600 1750
Wire Wire Line
	5050 2750 5600 2750
Wire Wire Line
	5600 2750 5600 1000
Wire Wire Line
	5600 1000 6200 1000
$Comp
L Connector_Generic:Conn_01x02 J6
U 1 1 5B71928D
P 8450 1200
F 0 "J6" H 8530 1192 50  0000 L CNN
F 1 "Conn_01x02" H 8530 1101 50  0000 L CNN
F 2 "Connector_Wire:SolderWirePad_1x02_P5.08mm_Drill1.5mm" H 8450 1200 50  0001 C CNN
F 3 "~" H 8450 1200 50  0001 C CNN
	1    8450 1200
	1    0    0    -1  
$EndComp
Wire Wire Line
	7000 1200 8250 1200
Wire Wire Line
	8250 1300 7000 1300
NoConn ~ 2600 2900
NoConn ~ 4050 2150
NoConn ~ 4050 2650
NoConn ~ 4050 2250
NoConn ~ 4050 2550
NoConn ~ 4050 2750
NoConn ~ 4050 2850
NoConn ~ 4050 2950
NoConn ~ 4050 3150
NoConn ~ 5050 3450
NoConn ~ 5050 3350
NoConn ~ 5050 3250
NoConn ~ 5050 3150
NoConn ~ 5050 3050
NoConn ~ 5050 2950
NoConn ~ 5050 2850
NoConn ~ 5050 2550
NoConn ~ 5050 2250
NoConn ~ 5050 2150
NoConn ~ 4650 1750
NoConn ~ 6200 1100
$Comp
L power:+12V #PWR0101
U 1 1 5C9D63F0
P 2100 1450
F 0 "#PWR0101" H 2100 1300 50  0001 C CNN
F 1 "+12V" H 2115 1623 50  0000 C CNN
F 2 "" H 2100 1450 50  0001 C CNN
F 3 "" H 2100 1450 50  0001 C CNN
	1    2100 1450
	1    0    0    -1  
$EndComp
Wire Wire Line
	2100 1450 2100 1750
$Comp
L power:GND #PWR0102
U 1 1 5C9DAC98
P 7600 2950
F 0 "#PWR0102" H 7600 2700 50  0001 C CNN
F 1 "GND" H 7605 2777 50  0000 C CNN
F 2 "" H 7600 2950 50  0001 C CNN
F 3 "" H 7600 2950 50  0001 C CNN
	1    7600 2950
	1    0    0    -1  
$EndComp
Wire Wire Line
	7600 2800 7600 2950
Wire Wire Line
	4650 3750 4650 3850
Wire Wire Line
	4650 3850 4550 3850
Connection ~ 4550 3850
Wire Wire Line
	4550 3850 4550 3900
$Comp
L power:PWR_FLAG #FLG0101
U 1 1 5C9DFAE5
P 4350 3900
F 0 "#FLG0101" H 4350 3975 50  0001 C CNN
F 1 "PWR_FLAG" H 4450 4100 50  0000 C CNN
F 2 "" H 4350 3900 50  0001 C CNN
F 3 "~" H 4350 3900 50  0001 C CNN
	1    4350 3900
	-1   0    0    1   
$EndComp
Wire Wire Line
	4350 3850 4550 3850
Wire Wire Line
	4350 3900 4350 3850
$Comp
L RelayModule:5V_1_Channel_Relay_Module U3
U 1 1 5CA61D0C
P 2950 2800
F 0 "U3" H 2950 3150 50  0000 C CNN
F 1 "5V_Relay_Module" H 2950 3050 50  0000 C CNN
F 2 "RelayModule:Relay_Module_1_Channel" H 2950 2800 50  0001 C CNN
F 3 "http://arduino.ua/prod2888-modyl-rele-5v-10a-visokogo-yrovnya-high-level" H 2950 2800 50  0001 C CNN
	1    2950 2800
	-1   0    0    1   
$EndComp
Wire Wire Line
	2400 2800 2600 2800
Wire Wire Line
	3300 2800 3500 2800
Wire Wire Line
	3500 2800 3500 3350
Wire Wire Line
	4750 1750 4750 1600
Wire Wire Line
	4750 1600 3400 1600
Wire Wire Line
	3400 1600 3400 2900
Wire Wire Line
	3400 2900 3300 2900
Connection ~ 4750 1750
Wire Wire Line
	1750 3250 2000 3250
Wire Wire Line
	1750 3150 2050 3150
Wire Wire Line
	1950 4100 1950 4200
Wire Wire Line
	1950 4000 1950 4100
Wire Wire Line
	1950 3350 1950 4000
Wire Wire Line
	1750 4000 2050 4000
Wire Wire Line
	1750 4100 2000 4100
$EndSCHEMATC
