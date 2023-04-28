# Revision History
## 2022-09-30
  - This code is based on USEN_220929
  - Enable AUTO_ONOFF_ENABLE under TAS5806MD_ENABLE
  - Changed TAS5806MD.c/TAS5806MD.h based on AD82584F.c/AD82584.h
  - Replaced all function of AD82584F.c to TAS5806MD around all files

## 2022-10-04
  - Added NOT_USE_POWER_DOWN_MUTE to use Deep Sleep mode instead of PD pin low under power off.  2022-10-04
  - To enable I2C communication under power off mode when we use TAS5806MD due to "Deep Sleep + Mute". 2022-10-04_1
  
## 2022-10-05
  - When Power On after Plug in, we need to init again after I2S LRCK detection. This is TAS5806 spec. //2022-10-05

## 2022-10-07
  - Port Setting for USEN_BAP //2022-10-07
  - Need to update volume level when we don't use LED_Display_Volume() //2022-10-27_1
  - Need to change LED port setting for USEN_BAP //2022-10-07_2
  - Implemented Power Key Feature //2022-10-07_3
  
## 2022-10-11
  - Separted TAS5806MD.c to two files(TAS5806MD_register.h) to move Init function. //2022-10-11_1
  - Implemented Interrupt Port E for BT Key(PE7) //2022-10-11_2
  - Need to Call clock setting function to use Timer1n //2022-10-11_3
  - To execute MB3021_BT_Master_Slave_Grouping_Stop under TAS5806MD_ENABLE //2022-10-11_4
  
## 2022-10-12
  - Implemented Master Volume & Attenuator Volume thru ADC feature. Please refer to ADC_INPUT_ENABLE //2022-10-12
  - Set Default Amp Mute after Amp init //2022-10-21_1
  - When we use DeepSleep mode under TAS5806MD, we don't need to amp init upon power on and just use TAS5806MD_Amp_Set_PWR_Control_Mode(TAS5806MD_PWR_Mode_PLAY) //2022-10-12_2
  - 1SPW MODE - bit1-0 : 01 //Need to use "1SPW Mode" over than 16V PVDD //2022-10-12_3
  - Implemented BT_UPDATE_DET //2022-10-12_4
  - Execute Mute On when BAP-01 goes power off mode under power off-->Power Plug Out/In because when power on, BAP-01 has audio noise slightly. //2022-10-12_5
  - Added Mute Off(TIMER20_mute_flag_Start()) in TAS5806MD_Amp_Detect_FS() function. //2022-10-12_6
  - Fixed Power_state value under Remocon_Power_Key_Action(Bool Power_on, Bool Slave_Sync, Bool Vol_Sync) //2022-10-12_7
  
## 2022-10-17
  - Implemented Auto Aux Switch(PA0) //2022-10-17_1
  - Changed from TAS5806MD_Amp_Detect_FS(void) to TAS5806MD_Amp_Detect_FS(Bool BInit) //2022-10-17_2
  - After Amp reset, need to init after detecting LRCK //2022-10-17_3
  - Fixed warning message //2022-10-17_4
  
## 2022-10-25
  - Implemented the FAULT PIN function of TAS8506MD //2022-10-25_1
  - Disable SOC_ERROR_ALAM in case of USEN_BAP

## 2022-10-26
  - Implemented AMP_ERROR_ALARM for TAS5806MD //2022-10-26
  - Fixed PF5 external interrupt typo
  
## 2022-10-28
  - Added SIG Test feature(Please refer to SIG_TEST).
  
## 2022-10-31(BSP-02)
  - When Amp error is ocurred, we don't need to alarm SOC_ERROR_MODE. //2022-10-31
  
## 2022-11-01(BSP-02)
  - When Amp error is ocurred, execute volume down actions. //2022-11-01
  
## 2022-11-03/2022-11-04 (BSP-02)
  - Implemented TWS Feature. Please refer to TWS_MODE_ENABLE //2022-11-03
  
## 2022-11-08 (BSP-02)
  - Implemented TWS communication Feature between Master and Slave. Please refer to TWS_MASTER_SLAVE_COM_ENABLE //2022-11-08

## 2022-11-09 (BSP-02)
  - Disable Remocon_Mode_Key_Action() under SW1_KEY pusing and then Execute BT Init //2022-11-09
  - Fixed TWS_MODE_ENABLE define error which is Master can't turn on BA mode(Broadcast) under last connection //2022-11-09_1
  
## 2022-11-11 (BSP-02)
  - Fixed the issue which MCU sends BA_SWITCH_MODE many time when last connection playing but this issue is closed when disable tWS_MODE_ENABLE(if(BMaster_Send_BLE_Remote_Data) //Master Mode Only is always FALSE)//2022-11-11
  - Fixed the issue which TAS5806MD can't not raised up Error Pin when Overtemperature after MUSIC pause and then play
  
## 2022-11-14 (BSP-02)
  - HW fixed the resistor value of TAS5806MD_I2C_ADDR to 15K ohm //2022-11-14_1
  - Need to Amp Init for Opened under Slave / TWS Mode //2022-11-14_2
  - Under TWS Mode, Master/Slave working is not permitted //2022-11-14_3
  - After Master/Slave connection, Sync Master status with Slave //2022-11-14_4
  
## 2022-11-15 (BSP-02)
  - To fix no audio or audio NG, Under TWS Slave mode, we don't use Clk_detect_uCount and will use clock detection timer to init Amp //2022-11-15_1
  - To fix no audio or audio NG, under TWS mode, we do't use Clock error detect anymore. //2022-11-15_2

## 2022-11-17 (BSP-02)  
  - Added some codes to set Master SPK as left channel(Slave should be Right channel after this command) //2022-11-17_1
  - Implemented the codes for supporting Aux mode //2022-11-17_2
  - the BBT_Is_Connected should be TRUE Under A2DP but not Under TWS. //2022-11-17_3

## 2022-11-22 (BSP-02)
  - Fixed Volume Level from HW team //2022-11-22
  - Implemented attenuator Action with 3-Step(Max/Mid/Min) //2022-11-22_1
  
## 2022-12-06 (BSP-02)
  - To delete audio noise(Slave abnormal volume) under TWS Mode, we keep audio mute untill CLK is stable //2022-12-06
  - Under connection with PC, TWS master mode working shuld be use this line when last connection //2022-12-06_1

## 2022-12-13 (BAP-01)
  - To Slave volume sync with Master Volume, Changed SW //2022-12-13
  
## 2022-12-14 (BAP-01)
  - Fixed volume curve for BAP-01 //2022-12-14
  
## 2022-12-15 (BSP-02)
  - Implemented the method of first TWS connection(Please refer to TWS_MASTER_SLAVE_GROUPING macro) //2022-12-15 
  
## 2022-12-20 (BAP-01)
  - Changed volume curve for BAP-01(from 0x58 to 0x4e) //2022-12-20_1
  - Modified Master/Slave Switch & Aux/BT Switch to work in inverse //2022-12-20_2
  
## 2022-12-20 (BSP-02)
  - When Peerdevice executes pause/play, slave SPK under TWS make stopping output due to Amp_Init for slave recovery. Changed SW that Just executes Recovery Init(Only first one time, it has I2S Clock) //2022-12-20_3

## 2022-12-22 (BSP-02)
  - Add cnodition BTWS_Master_Slave_Connect to avoid Master recognize peer device as slave after android device reboot when Master device updates tws slave address //2022-12-22

## 2022-12-23/2022-12-27 (BSP-02)
  - When user executes power plug out/in under general mode, BT SPK must keep general mode. If Last connection is failed, BT SPK should connect with other general device. Please refer to "BT_GENERAL_MODE_KEEP_ENABLE" //2022-12-23

## 2023-01-02 (BAP-01)
  - Deleted "To Slave volume sync with Master Volume, Changed SW //2022-12-13" because BSP-01 slave is working in inverse when BAP-01 Master volume is changed //2023-01-02_1
  - When Master is turned off and turn on by SSP communiication of USEN Tablet //2023-01-02_2
  - When Master is turned off and turn on by actual power key of USEN Tablet //2023-01-02_3  
  
## 2023-01-03 (BAP-01)
  - Turn on/off all LED by SSP communication on BAP-01 //2023-01-03_1
  - After we fixed 2023-01-03_1 using LED Power control, it makes BT short key interrupt because LED POWER CONTROL line is used as pull-up of BT KEY //2023-01-03_2

## 2023-01-04 (BAP-01)
  - Applied DSP volume control, 64-step volume control and 21 step attenuator control(Please refer to "ADC_VOLUME_64_STEP_ENABLE" and "TI_AMP_DSP_VOLUME_CONTROL_ENABLE") //2023-01-04
  
## 2023-01-05 (BAP-01)
  - FACTORY_RESET_KEY is invalid under BAP-01 Slave and only available it thru USEN Tablet SSP COM //2023-01-05_1
  - Attenuator action is inversed. So fixed it. //2023-01-05_2
  - BAP-01 Slave should be worked for Factory Reset Key thru BLE communication from BAP-01 Master //2023-01-05_3
  - To turning on all LED, when user press FACTORY RESET BUTTON //2023-01-05_4
  - BAP-01 can't support actual POWER KEY under Slave mode and it's just followed the stauts of Master //2023-01-05_5
  - To get/send 64 Step volume level under BAP //2023-01-05_6
  - Fixed that BSP-01 slave is working in inverse when BAP-01 Master volume is changed //2023-01-05_7

## 2023-01-06 (BAP-01)
  - Changed SW which send 64 step volume to BAP //2023-01-06_1
  - Need to send 16 step volume to Tablet under BAP //2023-01-06_2
  - To sync volume with BAP-01 Slave under MASTER_SLAVE_GROUPING and To send BAP-01 Volume Sync Data before GROUPING disconnection //2023-01-06_3
  
## 2023-01-09 (BAP-01)
  - To convert 64 step to 16 step volumem from BSP-01 under BAP //2023-01-09_1
  - To disable BLE_VOLUME_KEY using BLE DATA from Master under BAP slave when Master & Slave are BAP //2023-01-09_2
  - Under Slave Mode, Attenuator value is always +0dB and Volume is synchrinized with Master //2023-01-09_3
  - To Fit volume data from SSP/BLE under BAP //2023-01-09_3
  - Changed White Status LED working to follow USEN spec //2023-01-09_4
  
## 2023-01-10 (BAP-01)
  - When we send volume info to Slave, we need to keep original value to avoid wrong volume level of slave //2023-01-10_1
  - When BAP-01 is turned on using Power plug-in and USEN Tablet asks BAP-01 status using 0xBB, it sends wrong volume data //2023-01-10_2
  - Implemented Aux detect. Since Aux out is detected by interrupt, Timer has been checking if Aux detect is True during 20 Sec. Please refer to "AUX_DETECT_INTERRUPT_ENABLE". //2023-01-10_3
  
## 2023-01-11 (BAP-01)
  - Changed mute time from 2.5sec to 1sec //2023-01-11_1
  
## 2023-01-12 (BAP-01)
  - Changed volume level 1~4 by HW team //2023-01-12_1
  - Changed EQ_TOGGLE_ENABLE using MUTE KEY instead of BT KEY
  
## 2023-01-17 (BSP-02)
  - Implemented LED display of EQ Mode using MUTE KEY(Long Key - 3 sec) and save current EQ Mode on Flash. Please refer to PRIVATE_CUSTOM_MODE //2023-01-17
  
## 2023-01-18 (BSP-02)
  - To avoid click noise when Master is connected with Slave under TWS mode //2023-01-18_1(BSP-02 Issue #2)
  - To avoid mute when Master has audio from Aux but BT is changed from "streaming" to "suspend". //2023-01-18_2(BSP-02 Issue #5)
  
## 2023-01-20 (BSP-02)
  - To turn on audio when Master SPK works as standalone under TWS(Power Plug In case) //2023-01-20

## 2023-01-26 (BSP-02)
  - Under TWS mode, Master lose AD2P address and save TWS device as AD2P address when user plug-out slave. Fixed it. //2023-01-26_1
  - Under TWS Aux mode, Master can't output aux audio when user plug-out slave. Fixed it. //2023-01-26_2

## 2023-01-31 (BSP-02)
  - Implemented Alway General mode and do not use USEN mode. Please refer to "BT_ALWAYS_GENERAL_MODE" //2023-01-31_1
  - When Master don't have PLD List, we use fast white bliking LED display. //2023-01-31_2
  - When user push BT Long Key, Master should delete PDL List and work as General mode under enabling "BT_ALWAYS_GENERAL_MODE" //2023-01-31_3

## 2023-02-02 (BSP-02)
  - Fixed the bug of Alway General mode(Can't connect with USEN Tablet) //2023-02-02_1
  - Adjust Volume Level and EQ Setting on TI Amp //2023-02-02_2 this is only invalid under "USEN_230203(EJT_Biz_Trip_For_EQ_Tuning)" Folder 

## 2023-02-03 (BSP-02)
  - Need to make Mute ON when BT SPK disconnect with PeerDevice //2023-02-03_1

## 2023-02-06 (BAP-01)
  - If we use AUX_DETECT_INTERRUPT_ENABLE, we need to turn off BT LED under Aux Only Mode. //2023-02-06_1
  - Fixed that when plug out and plug in under Power off, BAP-01 has audio and led is turned on. //2023-02-06_2
  - Fixed that sometimes audio volume is up and dowwn even though it is same volume position. //2023-02-06_3

## 2023-02-08 (BAP-01)
  - Make Attenuator GAP //2023-02-08_1
  - Added additional code for Attenuator GAP //2023-02-08_2
  - Added additional code for Volume GAP //2023-02-08_3
  
## 2023-02-09 (BAP-01)
  - Ignored MUTE on LED display function //2023-02-09_1
  - When auto power off due to no music under power on state on BAP-01, user turn off using rotary switch on BAP-01. If user turn on using rotary switch on BAP-01, LED is not turned on. (BAP-01 Issue #10)
  - Changed Volume table for BSP-02 //2023-02-09_3

## 2023-02-10 (BAP-01)
  - Fixed the problem BAP-01 can't reconnect with peerdevice after disconnecting with peerdevice //2023-02-10_1
  - Fixed the problem BAP-01 can't connect with USEN Tablet //2023-02-10_2
  - Changed mute time from 1 sec to 1.5 sec due to ADC checking time reducing //2023-02-10_3
  
## 2023-02-14 (BAP-01)
  - When user makes power on with rotary switch after power plug Out --> In, this statement makes power on key is invalid key before finishing BT Init //2023-02-14_1 
  - Don't need LED display setting as STATUS_BT_GIA_PAIRING_MODE under slave mode //2023-02-14_2(USEN#1)
  
## 2023-02-15 (BSP-02)
  - When user push BT long key for 5sec after factory reset, BSP-02 works as factory reset. It's fixed. //2023-02-15_1 (USEN#7)
  
## 2023-02-16 (BSP-02)
  - Under TWS slave mode, slave keeps Blue LED On when user executes factory reset of Master. Accroding to spec, but Blue LED should be blinking. //2023-02-16_1(BSP-02 Issue #10)
  - To save TWS address and name even though A2DP_CONNECT_FAIL_ERRORCODE (Security Reject) //2023-02-16_2(Temp solution for BT EQ BYPASS FW) //Delete these solution. refer to 2023-02-20_1
  - To avoid pop noise on TWS Slave during init //2023-02-16_3

## 2023-02-20 (BSP-02)
  - To fix BT FW(2302170) bug under TWS Mode(Slave Name is "MB3021BNU0"). Remove 2023-02-16_2 //2023-02-20_1
  - Added recovery action for SET_DEVICE_ID sending and current information sending to TWS Slave to sync due to sync error, sometimes. //2023-02-20_2 (BSP-02 USEN#3/4)
  
## 2023-02-21 (BSP-02)
  - After Factory Reset or Reboot, SPK Should keep power stage(PLAY) under PLAY MODE and it do not set uRead = 0 //2023-02-21_1(BSP-02 USEN#3/4)
  - To recovery, SPK can't get TWS Address and Maaster sends sync data to Slave during Master is connected with Slave, only. //2023-02-21_2 (BSP-02 #3)
  - Fixed Mute Error after Volume Control. Need to Move General Page instead of DSP Page. //2023-02-21_3
  - Need to Set MODEL NAME as USEN MUSIC LINK under TWS Slave also. to avoid "2023-02-20_1" issue. //2023-02-21_4
  - We can't access TI AMP during TI AMP initializing / To aovid AMP access after boot on. //2023-02-21_5
  - Under TWS Mode, Master and Slave switch changed from LR to 360. Master is OK(It has Peerdevice's output) but Slave has no output(It need AMP reset). //2023-02-21_6
  - After reboot, Slave SPK has Audio NG issue. So, Added more delay. //2023-02-21_7
  - Add some more delay(10ms) for Master/Slave Key and LR/360 Key //2023-02-21_8
  
## 2023-02-21_1 (BAP-01)
  - Reduced Aux detect check time from 20 sec(2023-01-10_3) to 5 sec(Total 10sec = HW 5sec + SW 5sec)  //2023-02-21_9

## 2023-02-22 (BSP-02)
  - TWS Slave BT SPK executes Amp init again. Sometimes, BT SPK get this interrupt during Amp Init and Amp Init has wrong data. //2023-02-22_1
  - Disable AMP Init that is for AMP recovery(2022-11-14_4) //2023-02-22_2
  - After reboot, TWS Master sends data but TWS Slave ignore it due to AMP initializing. So, we need to send it again for recovery. //2023-02-22_3
  - Applied EJT EQ Setting Test 230214_001_OYM.h

## 2023-02-23 (BSP-02)
  - In BSP-01T Model, DSP Volume Level Tuning for EQ BYPASS BT FW. Max Volume = DSP(-8dB) + DAC(15.5dB) = 7.5 dB Max(DAC Gain Setting)//2023-02-23_1
  - In BSP-01T Model, Changed Default DAC GAIN(16.5dB - 1dB = 15.5dB) for EQ BYPASS BT FW //2023-02-23_2
  - Total -9dB reduction

## 2023-02-24 (BSP-02)
  - Applied EJT EQ Setting "230223_001_OYM.h"
  
## 2023-02-24_1 (BAP-01)
  - In BAP-01T Model, DSP Volume Level Tuning for EQ BYPASS BT FW. Max Volume = DSP(-8dB) + DAC(20.5dB) = 12.5 dB Max(DAC Gain Setting)//2023-02-24_1
  
## 2023-02-27 (BAP-01)
  - Use New USEN EQ Setting using TI AMP. Please refer to "USEN_IT_AMP_EQ_ENABLE". //2023-02-27_1
  - Added some codes to avoid interrupt during I2C commnunication. Please refer to BAmp_COM. //2023-02-27_2
  - Changed naming for Volume Step define from ADC_VOLUME_64_STEP_ENABLE to ADC_VOLUME_STEP_ENABLE.
  - Changed ADC volume step from 64 step to 50 step. //2023-02-27_3
  
## 2023-02-28 (BAP-01)
  - Changed volume table for converting. //2023-02-28_1
  - Under BAP-01 Slave Mode, When BAP-01 Master is connected, we need to disable Power Key. //2023-02-28_2(BAP-01 Issue #16)
  - Under BAP-01 Master Mode, if user changed rotary volume position very fast many times before BT init during Power on, Last connection is almost failed. //2023-02-28_3(BAP-01 Issue #14)
  
## 2023-03-02 (BAP-01 / BSP-02)
  - Send BAP-01 Name to USEN Tablet. //2023-03-02_1
  - Changed EQ and added DRC for BSP-02 //2023-03-02_2
  - Changed the concept of volume level update under BAP-01, we just use ADC value for Amp_Init() instead of flash data. //2023-03-02_3(BAP-01 Issue #14)

## 2023-03-03 (BSP-02)
  - For flash saving time of BT module during BT Long key action because BT SPK can do last connection, sometimes. //2023-03-03_1

## 2023-03-07 (BSP-02)
  - Applied Club EQ Setting only which is Semi-Final_02_230307 from OYM

## 2023-03-08 (BSP-02)
  - Sometimes, Some peerdevice send only MINOR_ID_ACL_CLOSED_IND on disconnection. so we need to add recovery code. //2023-03-08_1 (BSP-02 USEN#17, BSP-02 QA#6)
  - When user choose "Device Forget" and then "Device Delete" on peer device under TWS mode, user can't connect Master even though user choose Master in BT Menu. Added condtion to avoid sending TWS CMD again even though TWS Mode under Master Mode. //2023-03-08_2 (BSP-02 Issue #17, BSP-02 QA#5, BSP-02 QA#3)
  - Control volume level for each EQ Mode. //2023-03-08_3
  - Changed condition because Slave can't mute off after TWS pairing with Master and then rebooting even though Master is mute off after rebooting. //2023-03-08_4 (BSP-02 USEN#23)

## 2023-03-09 (BSP-02)
  - When user used Master with USEN Remote APP and then user changed the master to slave, we need to avoid USEN Remote APP try to connect with Slave using SPP. //2023-03-09_1 (BSP-02 Issue #19)
  - TWS can connect with USEN MUSIC LINK device Only. Sometimes If Peer Device has Slave address in its PDL, Peer device can connect with TWS Slave. //2023-03-09_2 (BSP-02 Issue #18)
  - When TWS mode is not ACL_CLOSED under TWS Slave mode, we don't need to make Mute On. //2023-03-09_3 (BSP-02 Issue #20)
  - When Master is mute state and then Power plug Out/In, Master can't keep mute on due to #ifdef error //2023-03-09_4 (BSP-02 QA#1)
  
## 2023-03-10 (BSP-02)
  - In i2c.c file, the read function need to change read size which depends on actual size from calling function. //2023-03-10_1
  - Changed concept of AMP clock checking to recover audio noise when I2S clock is not stable. //2023-03-10_2 (BSP-02 QA#9)
  - When TAS5806MD init, we need to add some delay for Data value but we don't set Data value before. //2023-03-10_3
  - Delete "2023-02-06-2" because Broadcast slave don't have audio when slave is powered off by power key and then plugged out/In. //2023-03-10_4 (BSP-02 QA#8)
  - When mute off, we need to check whether I2S Clock is stable or not. if I2S Clock is not stable, we use TIMER20_mute_flag_Start() instead of actual mute off. This is series of "2023-03-10_4". //2023-03-10_5 (BSP-02 QA#8, BSP-02 QA#2)
  - In TAS5806MD_Amp_Init(), we move to the position of TAS5806MD_Amp_Move_to_Control_Page() to protect I2C Access //2023-03-10_6
  - In TAS5806MD_Amp_Detect_Fault(), Added return condition to protect I2C Access //2023-03-10_7 (BSP-02 QA#9)
  
## 2023-03-13 (BSP-02)
  - Sometimes, BT Long Key is worked as Factory Reset due to factory reset recovery action. So, we need to keep BT Long Key action here.) //2023-03-13_1
  
## 2023-03-14 (BSP-02)
  - When user remove Power plug from Slave under TWS mode, Master sends sync data to slave for a while. it's fixed. //2023-03-14_1 (BSP-02 #3)
  - Under TWS first connection to avoid pop noise, we need to make mute on(w/o MUTE LED) before reboot. //2023-03-14_2 (BSP-02 #11)
  - The white status LED should be turned on alway under AUX IN Mode. //2023-03-14_3 (USEN #14)
  - To avoid HW noise of AMP, this is temporary. //2023-03-14_4
  
## 2023-03-14 (BAP-01)
  - Under Slave Mode, If User plug-in power cable when rotary power off and then power Plug-out, LED is turned on even though rotary power off. //2023-03-14_5
  
## 2023-03-15 (BSP-02)
  - Under TWS Mode, the Master doesn't send INFORM_HOST_MODE when Last conneection is OK. So, the Master has BT output even though it has aux input when Power plug-out/In. //2023-03-15_1 (BSP-02 #28)
  - When master is TWS Aux mode and BT is connecting with peer device, if user disconnect BT connection on peer device, MCU sends connection and discovery again & again, forever. So, we need to add condition to send discovery. //2023-03-15_2 (BSP-02 #29)
  - Added condition because if Master is Aux mode under TWS mode, we don't need make mute on also. //2023-03-15_3 (BSP-02 #30)
  - This case is only that TWS Master is connected with TWS Slave and PeerDevice is disconnected with TWS Master. We need to keep current TWS status. //2023-03-15_4 (BSP-02 #31)
  - Under TWS Mode, If user repeat connection/disconnection using BT List on Peer Device, sometimes, Slave LED is bliking and then Slave LED is on. //2023-03-15_5 (BSP-02 #33)
  - This case is only that TWS Master is connected with TWS Slave and PeerDevice is disconnected with TWS Master. We need to keep current TWS status. //2023-03-15_6 (BSP-02 #32)

## 2023-03-16 (BSP-02)
  - Changed max condition because this code erases FLASH_SAVE_SLAVE_LAST_CONNECTION info when user execute TWS connection between Master and Slave. //2023-03-16_1 (BSP-02 #34)
  - Add 2023-03-10_5 solution //2023-03-16_1 (BSP-02 #22)

## 2023-03-17 (BSP-02)
  - Added BT Long Key Action from USEN Tablet App using SPP. This key is only valid under Master. //2023-03-17_1

## 2023-03-23 (BAP-01)
  - Added EQ Setting from OYM under BAP-01. //2023-03-23_1

## 2023-03-27 (BAP-01)
  - USEN BAP-01 is only supporting Master mode. //2023-03-27_1
  - Implemented SW2_KEY action for BAP-01 EQ Setting. //2023-03-27_2
  - When BAP-01 is NORMAL Mode, we need to ignore EQ_KEY w/o error from USEN Tablet. //2023-03-27_3
  - Under BAP-01 NORMAL mode, BAP-01 can get only NORMAL MODE and send it to BSP-01 Slave. //2023-03-27_4

## 2023-03-28 (BAP-01)
  - Deleted sending extra data of BAP-01 due to changed spec which BAP-01 don't have BAP-01 Slave mode. //2023-03-28_1
  - Deleted receiving extra data of BAP-01 due to changed spec which BAP-01 don't have BAP-01 Slave mode. //2023-03-28_2
  - Send current information to USEN Tablet when user changes SW2_KEY(EQ NORMAL/EQ BSP) due to changed spec. //2023-03-28_3
  - Send current SW2_KEY status whether it's EQ NORMAL or EQ BSP due to chagned spec under BAP-01. //2023-03-28_4
  - When power on init, BAP-01 Master sned wrong volume data until user changed voluem thru rotary button or remote app. //2023-03-28_5
  - Added EQ NORMAL switch mode from EJT //2023-03-28_6

## 2023-03-29 (BSP-01)
  - When user disconnects BSP-01 in BT Menu on Peerdevice, If user executes power off->on over power button, Master has BT LED On(It should be blinking). //2023-03-29_1
  - BT Long key is working as factory reset. so, we need to fix this one.//2023-03-29_2

## 2023-03-30 (BSP-01) (BSP-02 USEN#15)
  - Disable 2023-03-15_6 solution for TWS Master. Because If user push BT short long key which TWS Master mode(Master is already connect with Slave) and then connect with other device, TWS slave lose audio output. //2023-03-30_1
  - When user push BT Long key under TWS Master mode, a speicific case make TWS slave is disconnected with Master. So, Added recovery solution when Slave get closed action on. This is solution for TWS Slave //2023-03-30_2
  - Under TWS Master, the SETIO_CAPABILITY_MODE will be worked only that TWS_Get_Slave_Name is executed. //2023-03-30_3

## 2023-04-03 (BSP-01)
  - For master mode checking of BAP-01 on factory line, we need to make BSP-01 Slave and it should be worked auto factory reset on disconnection with Master. //2023-04-03_1
  - When TWS slave is conected, we don't need to display BT STATUS LED on TWS Master. //2023-04-03_2 (BSP-02 #40)

## 2023-04-04 (BSP-01)
  - if the BT devcie is not cur A2DP under TWS Master, we don't need to apply it as peerdevice. //2023-04-04_1
  - When SW Reset after TWS connection, some statement should be worked at Master mode only. Because Slave can't reboot sometimes. //2023-04-04_2
  
## 2023-04-05 (BSP-01)
  - Move to here for initializing for static variable to fix switch action - SW2 & SW1. //2023-04-05_1 //To Do for BAP-01
  - Need to recognize SW1_KEY(360/LR) and SW2_KEY(Master/Slave) even though BT intial is not finished. //2023-04-05_2
  
## 2023-04-06 (BSP-01)
  - Added new define "LR_360_FACTORY_ENABLE" to follow new spec which when user switch 360 or LR, it should be worked with Factory Reset. //2023-04-06_1
  - To separate factory reset and BT HW reset which is action by Factory Reset, we make new function and codes. //2023-04-06_2
  - When user change the position of SW1 or SW2, Factory Resest is executed and LED Display All On() is called even though power off mode. //2023-04-06_3
  
## 2023-04-06 (BAP-01)
  - Under Power Off, when user changed Aux Auto/Fixed Switch and then power on using rotary key, we need to reflect this setting. //2023-04-06_4
 
## 2023-04-07 (BAP-01)
  - Added TAS5806MD Error condition and clock error recovery after first amp init. //2023-04-07_1
  - To recovery TAS5806MD_Amp_Detect_Fault() function  When we can't access TI amp (TAS5806MD_Amp_Detect_Fault()) becasue before amp init is not finished. //2023-04-07_2
  - Implemented AMP Error(Over temperature or Over current) action under BAP-01. //2023-04-07_3 (BAP-01 QA#1)
  
## 2023-04-12 (BAP-01)
  - We need to make FALSE because HW make 5sec delay to keep Aux In(Low) even though there is no Aux In after DC In. And we make Aux mode or BT mode after 5 sec. //2023-04-12_1 (BAP-01 QA HW#3/BAP-01 QA HW#9)
  - If this condition is not true, we'll retry again and again under mute off function. So, we don't need to send same mute off CMD to slave. //2023-04-12_2
  - Changed ADC checking speed from 100ms to 50msec //2023-04-12_3 (BAP-01 USEN#17)
  
## 2023-04-13 (BAP-01) : Changed Spe which BT short key is available for master/slave grouping under Aux mode
  - Under BAP-01, Aux mode shoud be accepted BT short key. //2023-04-13_1 (BAP-01 USEN#23/USEN#24)
  - Under Aux mode, when broadcast master/slave grouping, we need to make mute off. //2023-04-13_2 (BAP-01 USEN#23/USEN#24)
  - Changed BAP-01 Spec which BAP-01 should be supported LED display for Master/Slave grouping under aux mode also. //2023-04-13_3 (BAP-01 USEN#23/USEN#24)

## 2023-04-18 (BAP-01)
  - Added Grouping LED display condition under Aux Mode to avoid display LED under other case(BT connection disconnect from Peer Device and White LED is blinking). This is side effect from "2023-04-13" solution. //2023-04-18_1
  
## 2023-04-25 (BAP-01)
  - To keep EQ Mode for Amp_Init, we need to use cur_EQ_Mode under BAP-01. //2023-04-25_1

## 2023-04-26 (BSP-01T) - New TWS Connection
  - To make new spec which is new tws master slave link. Please refer to "NEW_TWS_MASTER_SLAVE_LINK". //2023-04-26_1
  - To make New TWS Connection, we need to skip GET_PAIRED_DEVICE_LIST(for Last connection) and execute SET_CONNECTABLE_MODE when first TWS connection under TWS master mode. //2023-04-26_2
  - To make New TWS Connection, we need to send TWS_CMD when first TWS connection under TWS master mode. //2023-04-26_3
  - To make New TWS Connection, we need to disable timer action to close TWS Master/Slave Grouping. //2023-04-26_4
  - To make New TWS Connection, we need to reset after 5sec since TWS Master sent SET_DEVICE_IDE to TWS Slave. //2023-04-26_5
  - To make New TWS Connection, we need to set BCRF_SET_DISCOVERABLE_MODE instead of BCRF_SET_LAST_CONNECTION when the last connection information is only TWS slave address. //2023-04-26_6
  - To make New TWS Connection, we need to reset after 5sec since TWS Slave get SET_DEVICE_IDE from TWS Master. //2023-04-26_7
  - To get DEVICE NAME over MINOR_ID_REMOTE_DEVICE_NAME_IND, Added TWS condition(During TWS Grouping / After TWS Grouping). //2023-04-26_8
  - To check whether SPK has the history of TWS connection or not. //2023-04-26_8
  - Changed TWS Satus sequence and Added some condition to set TWS_Status_Master_GIAC. //2023-04-26_9
  - When SPK get the information of TWS connection, we set "BTWS_Master_Slave_Connect = TWS_Get_Slave_Connected" for after TWS Grouping and "BTWS_Master_Slave_Connect = TWS_Get_Slave_Address" during TWS Grouping. //2023-04-26_10
  - Changed IO_CAPABILITY_MODE condition to check whether current device is TWS slave or not using uBT_TWS_Remote_Device_Name. //2023-04-26_11
  - Under TWS Mode, The agorithm of last connection is changed. //2023-04-26_12
  - If the address of closed device is A2DP, we need to make "BBT_Is_Connected = FALSE". //2023-04-26_13
  - TWS Master is connected with A2DP device and then TWS Master send LIAC to prohibit access from other devices. //2023-04-26_14
  - To make current status of Peer device(A2DP) and TWS Slave device. //2023-04-26_15
  - Added condition to avoid sending connectable mode and discoverable mode again and again. //2023-04-26_16
  - To save TWS Master address under TWS Slave mode. //2023-04-26_17
  - Changed conditon, to recover disconnection for TWS Slave. //2023-04-26_18
  - When TWS Slave is disconnected with TWS Master, it should display disconnection status using BT Status Blue LED. //2023-04-26_19
  - To play Aux under TWS Master need to send "BCRF_TWS_SET_DISCOVERABLE_MODE". //2023-04-26_20
  - Under TWS Mode, changed some condition for response of MINOR_ID_SET_DISCOVERABLE_MODE. //2023-04-26_21

## 2023-04-26_1 (BSP-01T)
  - When Power Off and Power on under Slave mode, Slave can't display Volume Level LED. This is side effect of //2023-04-06_3. //2023-04-26_22  
  
## 2023-04-27_1 (BAP-01)
  - Temparary SW Solution 500s check time to change Aux to BT. //2023-04-27_1
  
## 2023-04-27_2 (BSP-01T)
  - When TWS Master get reboot CMD over USEN Tablet remocon App, TWS Master need some delay to send reboot CMD to TWS Slave. //2023-04-27_2
  - After BT Long key on TWS Master, TWS Master can't connect with other device excepting previous peerdevice. //2023-04-27_3  
  
## 2023-04-28 (BAP-01)
  - Changed EQ Setting. Under BSP-EQ Mode, it's all same with BSP-01T EQ but Under EQ Normal, it's just keep previous EQ. //2023-04-28_1

## 2023-04-28 (BSP-01T)
  - Under BSP-01T broadcast mode, we need to return back to original code to avoid to send "BLE_SET_MANUFACTURER_DATA" when DC Power on. //2023-04-28_2
  - Changed Aux detection check delay from 40ms to 80ms to avoid undetection issue(Non-Aux case). //2023-04-28_3