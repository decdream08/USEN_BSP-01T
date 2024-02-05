/**********************************************************************
* @file		AD85050.c
* @brief	AMP
* @version	1.0
* @date		
* @author	SC Park
*
* Copyright(C) , ESTec
* All rights reserved.
*
**********************************************************************/

#include "main_conf.h"
#ifdef AD85050_ENABLE
#include "i2c.h"
#include "AD85050.h"
#if (defined(TIMER30_LED_PWM_ENABLE) && defined(TIMER1n_LED_PWM_ENABLE) || defined(TIMER21_LED_ENABLE))
#include "led_display.h"
#endif
#ifdef MB3021_ENABLE
#ifdef INPUT_KEY_SYNC_WITH_SLAVE_ENABLE
#include "bt_MB3021.h"
#endif
#endif
#if defined(AUTO_VOLUME_LED_OFF) || defined(SLAVE_ADD_MUTE_DELAY_ENABLE)
#include "timer20.h"
#endif
#ifdef FLASH_SELF_WRITE_ERASE
#include "flash.h"
#ifdef FLASH_SELF_WRITE_ERASE_EXTENSION
#include "remocon_action.h"
#endif
#endif //FLASH_SELF_WRITE_ERASE

/* Private typedef ---------------------------------------------------*/
/* Private define ----------------------------------------------------*/
#define AD85050_STATE_CTL6_REG						(0x1A)
#define AD85050_RESET_ON								(~(0x20))
#define AD85050_RESET_OFF								(0x20)

#define AD85050_DAC_GAIN_REG						(0x36)
#define AD85050_DAC_GAIN_MINUS_4DB					(0xC0)

#define AD85050_STATE_CTL3_REG						(0x02)
#define AD85050_MASTER_MUTE_ON						(0x40)
#define AD85050_MASTER_MUTE_OFF						(0x00)

//RAM Access Register
#define AD85050_COEFFICIENT_RAM_BASE_ADDR_REG		(0x1D)
#define AD85050_TOP_COFFICIENTS_A1_REG				(0x1E)
#define AD85050_MID_COFFICIENTS_A1_REG				(0x1F)
#define AD85050_BOTTOM_COFFICIENTS_A1_REG			(0x20)
#define AD85050_TOP_COFFICIENTS_A2_REG				(0x21)
#define AD85050_MID_COFFICIENTS_A2_REG				(0x22)
#define AD85050_BOTTOM_COFFICIENTS_A2_REG			(0x23)
#define AD85050_TOP_COFFICIENTS_B1_REG				(0x24)
#define AD85050_MID_COFFICIENTS_B1_REG				(0x25)
#define AD85050_BOTTOM_COFFICIENTS_B1_REG			(0x26)
#define AD85050_TOP_COFFICIENTS_B2_REG				(0x27)
#define AD85050_MID_COFFICIENTS_B2_REG				(0x28)
#define AD85050_BOTTOM_COFFICIENTS_B2_REG			(0x29)
#define AD85050_TOP_COFFICIENTS_A0_REG				(0x2A)
#define AD85050_MID_COFFICIENTS_A0_REG				(0x2B)
#define AD85050_BOTTOM_COFFICIENTS_A0_REG			(0x2C)
#define AD85050_RAM_SETTING_REG						(0x2D)

#define VOLUME_DEFAULT_LEVEL		(0x32)

#define AD85050_VOL_CONTROL_REG1					(0x03)
#define AD85050_ERROR_REG								(0x84)

uint8_t AD85050_Volume_Table[] = {
	0x00,	// MAX
  0x02,
  0x04,
  0x06,
  0x08,
  0x0a,
  0x0c,
  0x0e,
  0x10,
  0x12,
  0x14,
  0x16,
  0x18,
  0x1a,
  0x1c,
  0x1e,
  0x20,
  0x22,
  0x24,
  0x26,
  0x28,
  0x2a,
  0x2c,
  0x2e,
  0x30,
  0x32,
  0x34,
  0x36,
  0x38,
  0x3a,
  0x3c,
  0x3e,
  0x40,
  0x42,
  0x44,
  0x46,
  0x48,
  0x4a,
  0x4c,
  0x4e,
  0x50,
  0x52,
  0x54,
  0x56,
  0x58,
  0x5a,
  0x5c,
  0x5e,
  0x60,
	0xff	// Mute
};

#define MAX_VOLUME_LEVEL		(0)
#define VOLUME_LEVEL_NUMER	(sizeof(AD85050_Volume_Table))
#define MIN_VOLUME_LEVEL		(VOLUME_LEVEL_NUMER - 1)

/* Private macro -----------------------------------------------------*/
/* Private variables -------------------------------------------------*/
uint8_t uCurrent_Vol_Level = 0;
static Bool IS_Mute = FALSE;
#ifdef AD82584F_USE_POWER_DOWN_MUTE
static Bool Display_Mute = FALSE;
#endif
Bool volatile BAmp_Init = TRUE; //2023-02-21_5 : To aovid AMP access after boot on
Bool volatile BAmp_COM = FALSE; //2023-02-27_3 : To check whether AMP is busy(can't access - TRUE) or not(FALSE)

static EQ_Mode_Setting Cur_EQ_Mode = EQ_NORMAL_MODE;

void AD85050_Set_Cur_EQ_DRC_Mode(void)
{
	AD85050_Amp_EQ_DRC_Control(Cur_EQ_Mode);
}

Bool Is_BAmp_Init(void)
{
	Bool BRet = FALSE;
	
	if(BAmp_Init)
		BRet = TRUE;

	return BRet;
}

Bool Is_I2C_Access_OK(void) //2023-02-27_2 : Added some codes to avoid interrupt during I2C commnunication. Please refer to BAmp_COM(FALSE : Can't Access / TRUE : Can Access).
{
	Bool BRet = FALSE;

	if(Is_BAmp_Init() || BAmp_COM)
		BRet = FALSE;
	else
		BRet = TRUE;

	return BRet;	
}

Bool Is_Mute(void)
{
	Bool bRet;
	uint8_t uRead_Buf[FLASH_SAVE_DATA_END];

	//_DBG("\n\rIs_Mute() : ");
	
	Flash_Read(FLASH_SAVE_START_ADDR, uRead_Buf, FLASH_SAVE_DATA_END);
	//_DBH(uRead_Buf[FLASH_SAVE_DATA_MUTE]);
	if(uRead_Buf[FLASH_SAVE_DATA_MUTE] != 1)
		bRet = FALSE;
	else
		bRet = TRUE;

	return bRet;
}

void Set_Display_Mute(Bool B_Mute_On_Display) //2023-03-08_4 : For LED Display
{
#ifdef FLASH_SELF_WRITE_ERASE_EXTENSION
	uint8_t uRead_Buf[FLASH_SAVE_DATA_END];
	
	Flash_Read(FLASH_SAVE_START_ADDR, uRead_Buf, FLASH_SAVE_DATA_END);
#endif

	Display_Mute = B_Mute_On_Display;

	if(Display_Mute)
	{
#ifdef FLASH_SELF_WRITE_ERASE_EXTENSION
		if(uRead_Buf[FLASH_SAVE_DATA_MUTE] != 1)
			FlashSaveData(FLASH_SAVE_DATA_MUTE, 1); //Save mute on status to Flash
#endif
#ifdef TIMER21_LED_ENABLE //Set status led mode to Mute on
		Set_Status_LED_Mode(STATUS_MUTE_ON_MODE);
#endif	
	}
	else
	{
#ifdef FLASH_SELF_WRITE_ERASE_EXTENSION
		if(uRead_Buf[FLASH_SAVE_DATA_MUTE] != 0)
			FlashSaveData(FLASH_SAVE_DATA_MUTE, 0); //Save mute off status to Flash
#endif
#ifdef AUX_INPUT_DET_ENABLE
		if(Aux_In_Exist()) //Need to keep LED off under Aux Mode
			Set_Status_LED_Mode(STATUS_AUX_MODE);
		else
#endif
			Set_Status_LED_Mode(Get_Return_Status_LED_Mode());

	}
}

Bool IS_Display_Mute(void) //For LED Display
{
	return Display_Mute;
}

void Set_Is_Mute(Bool mute_on) //For Actual Mute Status
{
	IS_Mute = mute_on;
}

void AD85050_Amp_Init(Bool Power_On_Init)
{
  uint8_t uVol_Level = 0;
  uint8_t uCurVolLevel = 0;

	_DBG("\n\rAD85050_Amp_Init");

 	BAmp_Init = TRUE;

  delay_ms(20); //t7(20ms)
  AD85050_Amp_Reset(TRUE);
  delay_ms(20); //t8(20ms)
  AD85050_Amp_Reset(FALSE);  

	BAmp_Init = FALSE;

	//Master Volume Control
	//Data = 0x1f; //0x13; //1dB
	//I2C_Interrupt_Write_Data(AD85050_I2C_ADDR, 0x03,&Data,1);

  uVol_Level = ADC_Volume_Attenuator_Value_Init();
  
  AD85050_Dac_Volume_Set(FALSE);

  if(uVol_Level == 0xff)
  {
#ifdef _DBG_FLASH_WRITE_ERASE
    _DBG("\n\r#### Flash Data : First Init(Volume Level is 0xff)");
#endif
    uCurVolLevel = 64; //Level 64
    AD85050_Amp_Volume_Register_Writing(uCurVolLevel);
    AD85050_Amp_Set_Cur_Volume_Level(uCurVolLevel); //Save current volume level
  }
  else
  {
#ifdef _DBG_FLASH_WRITE_ERASE
    _DBG("\n\r#### Flash Data : Find Volume Level");
#endif
    AD85050_Amp_EQ_DRC_Control(Cur_EQ_Mode); //2023-05-02_2 : To keep current EQ mode under Amp init //(EQ_NORMAL_MODE); //DRC / EQ Setting

    if(Power_On_Init == TRUE)
      AD85050_Amp_Volume_Set_with_Index(uVol_Level, FALSE, TRUE); //Power On Init Call //2023-03-28_5 : Changed condition actual_key from FALSE to TRUE. When power on init, BAP-01 Master send wrong volume data until user changed voluem thru rotary button or remote app. Also, BAP-01 can use ACTUAL KEY in this case.
    else
      AD85050_Amp_Volume_Set_with_Index(uVol_Level, FALSE, TRUE); //Actual Key Input
  }

  delay_ms(10); //t9(10ms)
}

void AD85050_Amp_Reset(Bool Reset_On)
{
  uint8_t uRead = 0;

#ifdef AD82584F_DEBUG
  _DBG("\n\rAD82584F_Amp_Reset() : Reset_On = ");
  _DBD(Reset_On);
#endif

	BAmp_COM = TRUE;

  if(Reset_On)
  {
    I2C_Interrupt_Read_Data(AD85050_I2C_ADDR, AD85050_STATE_CTL6_REG,&uRead,1);
    uRead &= AD85050_RESET_ON;
    I2C_Interrupt_Write_Data(AD85050_I2C_ADDR, AD85050_STATE_CTL6_REG,&uRead,1);
  }
  else
  {
    I2C_Interrupt_Read_Data(AD85050_I2C_ADDR, AD85050_STATE_CTL6_REG,&uRead,1);
    uRead |= AD85050_RESET_OFF;
    I2C_Interrupt_Write_Data(AD85050_I2C_ADDR, AD85050_STATE_CTL6_REG,&uRead,1);
  }

	BAmp_COM = FALSE;
}

void AD85050_Amp_Mute(Bool Mute_On, Bool LED_Display) // First of all, You need to make sure if current page is Book 00 & Page00
{
	uint8_t uRead_Buf[FLASH_SAVE_DATA_END];
	uint8_t uRead = 0;

  if(!Power_State())
  {
#ifdef MUTE_CHECK_DEBUG_MSG	
    _DBG("\n\rPower Off mode : return ~!!!");
#endif		
    return;
  }

	if(LED_Display)
	  Flash_Read(FLASH_SAVE_START_ADDR, uRead_Buf, FLASH_SAVE_DATA_END);  

	BAmp_COM = TRUE;

	if(Mute_On) //Mute On
	{
#ifdef MUTE_CHECK_DEBUG_MSG	
		_DBG("\n\rMute On !!!");
#endif
		IS_Mute = TRUE;

		if(LED_Display)
		{
			Display_Mute = TRUE;

			if(uRead_Buf[FLASH_SAVE_DATA_MUTE] != 1)
				FlashSaveData(FLASH_SAVE_DATA_MUTE, 1); //Save mute on status to Flash
		}

		uRead = AD85050_MASTER_MUTE_ON;
		I2C_Interrupt_Write_Data(AD85050_I2C_ADDR, AD85050_STATE_CTL3_REG, &uRead,1);

		if(LED_Display)
			Set_Status_LED_Mode(STATUS_MUTE_ON_MODE);    
  }
  else
  {
#ifdef MUTE_CHECK_DEBUG_MSG	
    _DBG("\n\rMute Off !!!");
#endif
    IS_Mute = FALSE;

    if(LED_Display)
    {
      Display_Mute = FALSE;

      if(uRead_Buf[FLASH_SAVE_DATA_MUTE] != 0)
        FlashSaveData(FLASH_SAVE_DATA_MUTE, 0);
    }

		uRead = AD85050_MASTER_MUTE_OFF;
		I2C_Interrupt_Write_Data(AD85050_I2C_ADDR, AD85050_STATE_CTL3_REG, &uRead,1);    

  }

	if(LED_Display)
	{
		if(Aux_In_Exist()) //Need to keep LED off under Aux Mode
			Set_Status_LED_Mode(STATUS_AUX_MODE);
		else
			Set_Status_LED_Mode(Get_Return_Status_LED_Mode());
	}

 	BAmp_COM = FALSE;
}

void AD85050_Amp_Mute_Toggle(void) //Toggle
{
	uint8_t uRead = 0;

	uint8_t uRead_Buf[FLASH_SAVE_DATA_END];
	
	Flash_Read(FLASH_SAVE_START_ADDR, uRead_Buf, FLASH_SAVE_DATA_END);
	
	BAmp_COM = TRUE;
	
	if(uRead_Buf[FLASH_SAVE_DATA_MUTE] != 0x01)
	{
		IS_Mute = TRUE;
		Display_Mute = TRUE;
		
#ifdef MUTE_CHECK_DEBUG_MSG
		_DBG("\n\rMute On 1!!!");
#endif

		if(uRead_Buf[FLASH_SAVE_DATA_MUTE] != 1)
			FlashSaveData(FLASH_SAVE_DATA_MUTE, 1); //Save mute on status to Flash

		uRead = AD85050_MASTER_MUTE_ON;
		I2C_Interrupt_Write_Data(AD85050_I2C_ADDR, AD85050_STATE_CTL3_REG, &uRead,1);
		
		Set_Status_LED_Mode(STATUS_MUTE_ON_MODE);
	}
	else
	{
		IS_Mute = FALSE;
		Display_Mute = FALSE;
		
#ifdef MUTE_CHECK_DEBUG_MSG	
    _DBG("\n\rMute Off for Actual Mute Key - Actual Mute Off !!!");
#endif

		if(uRead_Buf[FLASH_SAVE_DATA_MUTE] != 0)
			FlashSaveData(FLASH_SAVE_DATA_MUTE, 0); //Save mute off status to Flash

		uRead = AD85050_MASTER_MUTE_OFF;
		I2C_Interrupt_Write_Data(AD85050_I2C_ADDR, AD85050_STATE_CTL3_REG, &uRead,1);

		if(Aux_In_Exist()) //Need to keep LED off under Aux Mode
			Set_Status_LED_Mode(STATUS_AUX_MODE);
		else
			Set_Status_LED_Mode(Get_Return_Status_LED_Mode());
	}

	if(IS_Mute)
		MB3021_BT_Module_Input_Key_Sync_With_Slave(input_key_Sync_Mute, 0x01);
	else
		MB3021_BT_Module_Input_Key_Sync_With_Slave(input_key_Sync_Mute, 0x00);

	BAmp_COM = FALSE;
}

uint8_t AD85050_Amp_Volume_Set_with_Index(uint8_t Vol_Level, Bool Inverse, Bool Actual_Key) //Actual Key says this is not SSP or BLE communication. So, we need to send same key to Slave SPK
{
  uint8_t uCurVolLevel = 0;

	if(((Vol_Level > VOLUME_LEVEL_NUMER) && Inverse) || ((Vol_Level > (VOLUME_LEVEL_NUMER-1)) && !Inverse))
  {
    return 0xff;
  }

	if(Inverse)
	{
		uCurVolLevel = VOLUME_LEVEL_NUMER - Vol_Level; //Input : 50~1 / Output : 0 ~ 49(Actual Volume Table)
	}
	else
		uCurVolLevel = Vol_Level;

	BAmp_COM = TRUE;
  AD85050_Amp_Volume_Register_Writing(uCurVolLevel);
	BAmp_COM = FALSE;
  
	AD85050_Amp_Set_Cur_Volume_Level(uCurVolLevel); //Save current volume level  

	if(Actual_Key)
	{
		MB3021_BT_Module_Input_Key_Sync_With_Slave(input_key_Sync_Volume, (VOLUME_LEVEL_NUMER-1) - Vol_Level);
	}  

	return uCurVolLevel;
}

// First of all, You need to make sure if current page is Book 00 & Page00
void AD85050_Amp_Volume_Control(Vol_Setting Vol_mode) //Volume Up/Down
{
	uint8_t uCurVolLevel = 0;
	static uint8_t uCount = 0;
  
  uCurVolLevel = AD85050_Amp_Get_Cur_Volume_Level();

  if(Vol_mode == Volume_Up)
    uCurVolLevel-=1;
  else
    uCurVolLevel+=1;

  if(uCurVolLevel > MIN_VOLUME_LEVEL)
  {
    if(uCount%2)
      MB3021_BT_Module_Input_Key_Sync_With_Slave(Input_key_Sync_Slave_Mute_Off, 0x03);
    else
      MB3021_BT_Module_Input_Key_Sync_With_Slave(Input_key_Sync_Slave_Mute_Off, 0x04);

    uCount++;

    return;
  }

	BAmp_COM = TRUE;
	AD85050_Amp_Volume_Register_Writing(uCurVolLevel);
	BAmp_COM = FALSE;

	AD85050_Amp_Set_Cur_Volume_Level(uCurVolLevel); //Save current volume level
	MB3021_BT_Module_Input_Key_Sync_With_Slave(input_key_Sync_Volume, (VOLUME_LEVEL_NUMER-1) - uCurVolLevel);
}

void AD85050_Amp_RAM_Single_Write(uint8_t uCount, uint8_t uData)
{
	uint8_t Data = 0, count = 0;
#ifdef AD85050_DEBUG_MSG
	_DBG("\n\rAD85050_Amp_RAM_Single_Write() !!!");
#endif

	Data = uData;
	count = uCount%5;
			
	switch(count)
	{
		case 0:
			I2C_Interrupt_Write_Data(AD85050_I2C_ADDR, AD85050_COEFFICIENT_RAM_BASE_ADDR_REG,&Data,1);
			break;
		case 1:
			I2C_Interrupt_Write_Data(AD85050_I2C_ADDR, AD85050_TOP_COFFICIENTS_A1_REG,&Data,1);
			break;
		case 2:
			I2C_Interrupt_Write_Data(AD85050_I2C_ADDR, AD85050_MID_COFFICIENTS_A1_REG,&Data,1);
			break;
		case 3:
			I2C_Interrupt_Write_Data(AD85050_I2C_ADDR, AD85050_BOTTOM_COFFICIENTS_A1_REG,&Data,1);
			break;
		case 4:
			I2C_Interrupt_Write_Data(AD85050_I2C_ADDR, AD85050_RAM_SETTING_REG,&Data,1);
			break;
		default:
			break;
	}
}

void AD85050_Amp_RAM_Three_Coeff_Write(uint8_t uCount, uint8_t uData)
{
	uint8_t Data = 0, count = 0;
#ifdef AD85050_DEBUG_MSG
		_DBG("\n\rAD85050_Amp_RAM_Three_Coeff_Write() !!!");
#endif

	Data = uData;
	count = uCount%11;

	switch(count)
	{
		case 0:
			I2C_Interrupt_Write_Data(AD85050_I2C_ADDR, AD85050_COEFFICIENT_RAM_BASE_ADDR_REG,&Data,1);
			break;
		case 1:
			I2C_Interrupt_Write_Data(AD85050_I2C_ADDR, AD85050_TOP_COFFICIENTS_A1_REG,&Data,1);
			break;
		case 2:
			I2C_Interrupt_Write_Data(AD85050_I2C_ADDR, AD85050_MID_COFFICIENTS_A1_REG,&Data,1);
			break;
		case 3:
			I2C_Interrupt_Write_Data(AD85050_I2C_ADDR, AD85050_BOTTOM_COFFICIENTS_A1_REG,&Data,1);
			break;
		case 4:
			I2C_Interrupt_Write_Data(AD85050_I2C_ADDR, AD85050_TOP_COFFICIENTS_A2_REG,&Data,1);
			break;
		case 5:
			I2C_Interrupt_Write_Data(AD85050_I2C_ADDR, AD85050_MID_COFFICIENTS_A2_REG,&Data,1);
			break;
		case 6:
			I2C_Interrupt_Write_Data(AD85050_I2C_ADDR, AD85050_BOTTOM_COFFICIENTS_A2_REG,&Data,1);
			break;
		case 7:
			I2C_Interrupt_Write_Data(AD85050_I2C_ADDR, AD85050_TOP_COFFICIENTS_B1_REG,&Data,1);
			break;
		case 8:
			I2C_Interrupt_Write_Data(AD85050_I2C_ADDR, AD85050_MID_COFFICIENTS_B1_REG,&Data,1);
			break;
		case 9:
			I2C_Interrupt_Write_Data(AD85050_I2C_ADDR, AD85050_BOTTOM_COFFICIENTS_B1_REG,&Data,1);
			break;
		case 10:
			I2C_Interrupt_Write_Data(AD85050_I2C_ADDR, AD85050_RAM_SETTING_REG,&Data,1);
			break;
		default:
			break;
	}
}

void AD85050_Amp_RAM_Set_Write(uint8_t uCount, uint8_t uData)
{
	uint8_t Data = 0, count = 0;
#ifdef AD85050_DEBUG_MSG
		_DBG("\n\rAD85050_Amp_RAM_Set_Write() !!!");
#endif

	Data = uData;
	count = uCount%17;

	switch(count)
	{
		case 0:
			I2C_Interrupt_Write_Data(AD85050_I2C_ADDR, AD85050_COEFFICIENT_RAM_BASE_ADDR_REG,&Data,1);
			break;
		case 1:
			I2C_Interrupt_Write_Data(AD85050_I2C_ADDR, AD85050_TOP_COFFICIENTS_A1_REG,&Data,1);
			break;
		case 2:
			I2C_Interrupt_Write_Data(AD85050_I2C_ADDR, AD85050_MID_COFFICIENTS_A1_REG,&Data,1);
			break;
		case 3:
			I2C_Interrupt_Write_Data(AD85050_I2C_ADDR, AD85050_BOTTOM_COFFICIENTS_A1_REG,&Data,1);
			break;
		case 4:
			I2C_Interrupt_Write_Data(AD85050_I2C_ADDR, AD85050_TOP_COFFICIENTS_A2_REG,&Data,1);
			break;
		case 5:
			I2C_Interrupt_Write_Data(AD85050_I2C_ADDR, AD85050_MID_COFFICIENTS_A2_REG,&Data,1);
			break;
		case 6:
			I2C_Interrupt_Write_Data(AD85050_I2C_ADDR, AD85050_BOTTOM_COFFICIENTS_A2_REG,&Data,1);
			break;
		case 7:
			I2C_Interrupt_Write_Data(AD85050_I2C_ADDR, AD85050_TOP_COFFICIENTS_B1_REG,&Data,1);
			break;
		case 8:
			I2C_Interrupt_Write_Data(AD85050_I2C_ADDR, AD85050_MID_COFFICIENTS_B1_REG,&Data,1);
			break;
		case 9:
			I2C_Interrupt_Write_Data(AD85050_I2C_ADDR, AD85050_BOTTOM_COFFICIENTS_B1_REG,&Data,1);
			break;
		case 10:
			I2C_Interrupt_Write_Data(AD85050_I2C_ADDR, AD85050_TOP_COFFICIENTS_B2_REG,&Data,1);
			break;
		case 11:
			I2C_Interrupt_Write_Data(AD85050_I2C_ADDR, AD85050_MID_COFFICIENTS_B2_REG,&Data,1);
			break;
		case 12:
			I2C_Interrupt_Write_Data(AD85050_I2C_ADDR, AD85050_BOTTOM_COFFICIENTS_B2_REG,&Data,1);
			break;
		case 13:
			I2C_Interrupt_Write_Data(AD85050_I2C_ADDR, AD85050_TOP_COFFICIENTS_A0_REG,&Data,1);
			break;
		case 14:
			I2C_Interrupt_Write_Data(AD85050_I2C_ADDR, AD85050_MID_COFFICIENTS_A0_REG,&Data,1);
			break;
		case 15:
			I2C_Interrupt_Write_Data(AD85050_I2C_ADDR, AD85050_BOTTOM_COFFICIENTS_A0_REG,&Data,1);
			break;
		case 16:
			I2C_Interrupt_Write_Data(AD85050_I2C_ADDR, AD85050_RAM_SETTING_REG,&Data,1);
			break;
		default:
			break;
	}
}

void AD85050_Amp_Mode_Control(Audio_Output_Setting mode) //To Do !!!
{
#if 0
	uint8_t Data = 0, i;

#ifdef TAS5806MD_DEBUG_MSG
	_DBG("\n\rTAS5806MD_Amp_Mode_Control() !!! mode =");
	_DBD(mode);
#endif

	TAS5806MD_Amp_Move_to_Control_Page();

	switch(mode)
	{
		case LL_MODE:
			for(i=0; i<20; i++)
			{
				Data = LR_Mode_Ctl_Table[LL_MODE][i];
				AD82584F_Amp_RAM_Single_Write(i, Data);
			}
			break;
		case RR_MODE:
			for(i=0; i<20; i++)
			{
				Data = LR_Mode_Ctl_Table[RR_MODE][i];
				AD82584F_Amp_RAM_Single_Write(i, Data);
			}
			break;
		case STEREO_MODE:
			for(i=0; i<20; i++)
			{
				Data = LR_Mode_Ctl_Table[STEREO_MODE][i];
				AD82584F_Amp_RAM_Single_Write(i, Data);
			}
			break;
		default:
			break;
	}
#endif
}

void AD85050_Amp_EQ_DRC_Control(EQ_Mode_Setting EQ_mode)
{
	uint8_t uCurVolLevel;
	uint8_t uData = 0, uCount = 0, uSize = 0;

#ifdef AD85050_DEBUG_MSG
	_DBG("\n\r+++ AD85050_Amp_EQ_DRC_Control() !!!!");
#endif

  if(Is_BAmp_Init()) //2023-02-22_1
  {
    TIMER20_drc_eq_set_flag_start();
    return;
  }

	TIMER20_drc_eq_set_flag_stop();  

	BAmp_COM = TRUE;
	Cur_EQ_Mode = EQ_mode;

  switch(EQ_mode)
  {
		case EQ_NORMAL_MODE:
      break;

    case EQ_POP_ROCK_MODE:
      break;
      
    case EQ_CLUB_MODE:
      break;
            
    case EQ_JAZZ_MODE:
      break;

		case EQ_VOCAL_MODE:
      break;
      
    default:
      break;
  }
  
	uCurVolLevel = AD85050_Amp_Get_Cur_Volume_Level();
	AD85050_Amp_Volume_Register_Writing(uCurVolLevel);

	MB3021_BT_Module_Input_Key_Sync_With_Slave(input_key_Sync_EQ, EQ_mode);  

	BAmp_COM = FALSE;  
}

void AD85050_Amp_Set_Cur_Volume_Level(uint8_t volume)
{
#ifdef AD85050_DEBUG_MSG
		_DBG("\n\rAD85050_Amp_Set_Cur_Volume_Level() : volume =");
		_DBD(volume);
#endif

	uCurrent_Vol_Level = volume;
}

uint8_t AD85050_Amp_Get_Cur_Volume_Level(void) //Start count from Max(15)
{
#ifdef AD85050_DEBUG_MSG
		_DBG("\n\rAD85050_Amp_Get_Cur_Volume_Level() : volume =");
		_DBD(uCurrent_Vol_Level);
#endif

	return uCurrent_Vol_Level;
}

uint8_t AD85050_Amp_Get_Cur_Volume_Level_Inverse(void) //Start count from Min(0)
{
	uint8_t uInverse_Vol;
#ifdef AD85050_DEBUG_MSG
		_DBG("\n\rAD85050_Amp_Get_Cur_Volume_Level_Inverse() : volume =");
		_DBD(uCurrent_Vol_Level);
#endif
	uInverse_Vol = (VOLUME_LEVEL_NUMER-1) -uCurrent_Vol_Level;

	return uInverse_Vol;
}

Bool AD85050_Amp_Get_Cur_CLK_Status(void) //TRUE : Clock Exist / FALSE : Clock absence
{	
	uint8_t uRead = 0;
	Bool Ret;
	
	I2C_Interrupt_Read_Data(AD85050_I2C_ADDR, AD85050_ERROR_REG, &uRead, 1);

#ifdef AD85050_DEBUG
	_DBG("\n\rRead_Amp_Data : ");
	_DBH(uRead);
#endif

	if(uRead & 0x04) //Bit 2 - 0(CKERR occur - None) / 1(Normal)
		Ret = TRUE;
	else
		Ret = FALSE;
	
	return Ret;
}

void AD85050_Amp_Set_Default_Volume(void)
{
}

Bool AD85050_Amp_Detect_FS(Bool BInit) //Detect Audio Sampling Feq //2022-10-05 //2022-10-17_2
{
	return TRUE;
}

uint8_t AD85050_Amp_Detect_Fault(Bool Return_Val_Only) //2022-10-25 : FAULT PIN //2022-11/10 : Changed return from Bool to uint8_t(B_Error_Flag = 1:HighTemp/2:other/0:None)
{	
#if 1 //DDD...TEST
	return 0;
#else //DDD...TEST

	//Bool B_Is_Error = FALSE;
	uint8_t Ch_Fault = 0, Fault1 = 0, Fault2 = 0, Warning = 0,  B_Error_Flag = 0;
#ifdef USEN_BAP //2023-04-07_3
	uint8_t Volume_Level = 0;
#endif

	if(Is_BAmp_Init() || Is_I2C_Access_OK() == FALSE) //2023-03-10_7  //2023-02-21_5 : To avoid AMP access after boot on
	{
#ifdef TAS5806MD_DEBUG_MSG
		_DBG("\n\r+++ Is_BAmp_Init is TRUE - 2");
#endif
		return 0xff; //2023-04-07_2 : To retry for fault detect
	}

	BAmp_COM = TRUE; //2023-03-10_7 //FALSE;

	TAS5806MD_Amp_Move_to_Control_Page();
	
	I2C_Interrupt_Read_Data(TAS5806MD_I2C_ADDR, TAS5806MD_CHAN_FAULT_REG,&Ch_Fault,1);	
#if 0
	if(Ch_Fault & 0xF)
		B_Is_Error = TRUE;
#endif	
	if(Ch_Fault & 0x8)
	{
#ifdef COMMON_DEBUG_MSG
		_DBG("\n\r !!!!! Left Channel DC fault");
#endif
		B_Error_Flag = 1; //2023-04-07_1 : Added TAS5806MD Error condition
	}

	if(Ch_Fault & 0x4)
	{
#ifdef COMMON_DEBUG_MSG
		_DBG("\n\r !!!!! Right Channel DC fault");
#endif
		B_Error_Flag = 1; //2023-04-07_1 : Added TAS5806MD Error condition
	}

	if(Ch_Fault & 0x2)
	{
#ifdef COMMON_DEBUG_MSG
		_DBG("\n\r !!!!! Left Channel over current fault");
#endif
		B_Error_Flag = 1; //2023-04-07_1 : Added TAS5806MD Error condition
	}

	if(Ch_Fault & 0x1)
	{
#ifdef COMMON_DEBUG_MSG
		_DBG("\n\r !!!!! Right Channel over current fault");
#endif
		B_Error_Flag = 1; //2023-04-07_1 : Added TAS5806MD Error condition
	}

	I2C_Interrupt_Read_Data(TAS5806MD_I2C_ADDR, TAS5806MD_GLOBAL_FAULT1_REG,&Fault1,1);
#if 0
	if(Fault1 & 0xC7)
		B_Is_Error = TRUE;
#endif

	if(Fault1 & 0x80)
	{
#ifdef COMMON_DEBUG_MSG
		_DBG("\n\r !!!!! Indicate OTP CRC check error");
#endif
	}

	if(Fault1 & 0x40)
	{
#ifdef COMMON_DEBUG_MSG
		_DBG("\n\r !!!!! The recent BQ is written failed");
#endif
	}

	if(Fault1 & 0x4)
	{
#ifdef COMMON_DEBUG_MSG
		_DBG("\n\r !!!!! Clock fault");
#endif

#if 0 //2023-05-02_3 : Need to disable to avoid amp init(2023-04-07_1) when Tablet move to next song(Suspend --> Play)
		if(TAS5806MD_CLK_Detect_Count() == 0xffffffff) //2023-04-07_1 : Added Clock Error Recovery after first AMP init.
		{
#ifdef COMMON_DEBUG_MSG
			_DBG("\n\r !!!!! Clock fault - Recovery");
#endif
			TAS5806MD_Init_After_Clk_Detect();
		}
#endif
	}

	if(Fault1 & 0x2)
	{
#ifdef COMMON_DEBUG_MSG
		_DBG("\n\r !!!!! PVDD OV fault");
#endif
	}
	
	if(Fault1 & 0x1)
	{
#ifdef COMMON_DEBUG_MSG
		_DBG("\n\r !!!!! PVDD UV fault");
#endif
	}

	I2C_Interrupt_Read_Data(TAS5806MD_I2C_ADDR, TAS5806MD_GLOBAL_FAULT2_REG,&Fault2,1);

	if(Fault2 & 0x1)
	{
#ifdef COMMON_DEBUG_MSG
		_DBG("\n\r !!!!! Over temperature shut down fault");
#endif
		B_Error_Flag = 1;
		//B_Is_Error = TRUE;
	}
	
	I2C_Interrupt_Read_Data(TAS5806MD_I2C_ADDR, TAS5806MD_OT_WARNING_REG,&Warning,1);

	if(Warning & 0x4)
	{
#ifdef COMMON_DEBUG_MSG
		_DBG("\n\r !!!!! Over temperature warning ,135C");
#endif
		B_Error_Flag = 1;
		//B_Is_Error = TRUE;
	}

	if(B_Error_Flag == 1) //1:HighTemp
	{	
#if defined(TIMER21_LED_ENABLE) && defined(AMP_ERROR_ALARM)
		if(Get_Cur_Status_LED_Mode() != STATUS_AMP_ERROR_MODE)
			Set_Status_LED_Mode(STATUS_AMP_ERROR_MODE); //over-temperature or short-circuit condition
#endif

#if defined(AMP_ERROR_ALARM) || (defined(SOC_ERROR_ALARM) && defined(TAS5806MD_ENABLE)) //2022-11-01
		if(!Return_Val_Only)
		{
#ifdef USEN_BAP
			//2023-04-07_3 : Need to execute Hi-Temp Error (1 : Related High-Temp / 2 : Clock)
			TAS5806MD_Fault_Clear_Reg(); //Added Fault Clear here to output audio w/o stopping
#ifdef COMMON_DEBUG_MSG
			_DBG("\n\r+++ Forced volume Down");
#endif
			Volume_Level = TAS5806MD_Amp_Get_Cur_Volume_Level();
			Volume_Level += 10;
#ifdef TI_AMP_DSP_VOLUME_CONTROL_ENABLE
			TAS5806MD_Amp_Volume_Register_Writing(Volume_Level);
			TIMER20_Amp_error_flag_Start(); //For LED Display - Only Clear this variable on Amp init
#endif
			//TAS5806MD_AGL_Value_Change(); //2023-04-07_3 : Disable
#else //USEN_BAP
#ifdef SWITCH_BUTTON_KEY_ENABLE
			Send_Remote_Key_Event(VOL_DOWN_KEY);
#endif
#endif //USEN_BAP
			TAS5806MD_Fault_Clear_Reg(); //Added Fault Clear here to output audio w/o stopping
			TIMER20_Amp_error_flag_Start();
		}
#endif //#if defined(AMP_ERROR_ALARM) || (defined(SOC_ERROR_ALARM) && defined(TAS5806MD_ENABLE))
	}
	else
	{
#if 0 //2022-11-15_2 : This statement make Amp init error probably
		if(B_Is_Error)
		{
			B_Error_Flag = 2;
			TIMER20_Amp_error_flag_Start();
		}
		else
#endif
			B_Error_Flag = 0;

#ifdef TAS5806MD_ENABLE //2023-07-06_1 : Applied this solution(2023-06-30_1) under BSP-01T //2023-06-30_1
		TIMER20_Amp_error_no_diplay_flag_Start();
#endif
	}

	BAmp_COM = FALSE;

	return B_Error_Flag;
	//To Do !!! Need to Add Mute or Volume down functions here
#endif //DDD...TEST
}

void AD85050_Register_Read(void) //2022-10-25 : FAULT PIN
{
}

void AD85050_Fault_Clear_Reg(void) 
{
#if 1 //DDD...TEST
	return;
#else //DDD...TEST

	uint8_t uWrite = 0;

#ifdef TAS5806MD_DEBUG_MSG
	_DBG("\n\rTAS5806MD_Fault_Clear_Reg()");
#endif
	BAmp_COM = TRUE;

	uWrite = 0x80; //Bit 7 : Write Clear Bit
	I2C_Interrupt_Write_Data(TAS5806MD_I2C_ADDR, TAS5806MD_FAULT_CLEAR_REG,&uWrite,1);

	BAmp_COM = FALSE;
#endif //DDD...TEST
}

void AD85050_AGL_Value_Change(Switch_BAP_EQ_Mode EQ_Mode, Bool BT_mode) //2023-06-19_1 : Added parameter in TAS5806MD_AGL_Value_Change to apply AGL for each mode(BT or Aux)
{
#if 1 //DDD...TEST
	return;
#else //DDD...TEST

	uint8_t uSize, i, Data;
	
#ifdef TAS5806MD_DEBUG_MSG
	_DBG("\n\rTAS5806MD_AGL_Value_Change()");
#endif
	//Write TAS5806MD_Init	

	if(BT_mode) //BT Mode
	{
		if(EQ_Mode == Switch_EQ_BSP_Mode) //BT Mode : BSP Mode
		{
			uSize = sizeof(TAS5806MD_AGL_EQ_BSP_BT)/2;

			for(i =0;i<uSize;i++)
			{
				Data = TAS5806MD_AGL_EQ_BSP_BT[i][1];
				I2C_Interrupt_Write_Data(TAS5806MD_I2C_ADDR, TAS5806MD_AGL_EQ_BSP_BT[i][0],&Data,1);
			}
		}
		else //BT Mode : NORMAL Mode
		{
			uSize = sizeof(TAS5806MD_AGL_EQ_NORMAL_BT)/2;

	for(i =0;i<uSize;i++)
	{
				Data = TAS5806MD_AGL_EQ_NORMAL_BT[i][1];
				I2C_Interrupt_Write_Data(TAS5806MD_I2C_ADDR, TAS5806MD_AGL_EQ_NORMAL_BT[i][0],&Data,1);
			}
		}
	}
	else //Aux Mode
	{
		if(EQ_Mode == Switch_EQ_BSP_Mode) //Aux Mode : BSP Mode
		{
			uSize = sizeof(TAS5806MD_AGL_EQ_BSP_AUX)/2;

			for(i =0;i<uSize;i++)
			{
				Data = TAS5806MD_AGL_EQ_BSP_AUX[i][1];
				I2C_Interrupt_Write_Data(TAS5806MD_I2C_ADDR, TAS5806MD_AGL_EQ_BSP_AUX[i][0],&Data,1);
			}
		}
		else //Aux Mode : NORMAL Mode
		{
			uSize = sizeof(TAS5806MD_AGL_EQ_NORMAL_AUX)/2;

			for(i =0;i<uSize;i++)
			{
				Data = TAS5806MD_AGL_EQ_NORMAL_AUX[i][1];
				I2C_Interrupt_Write_Data(TAS5806MD_I2C_ADDR, TAS5806MD_AGL_EQ_NORMAL_AUX[i][0],&Data,1);
			}
		}
	}
#endif //DDD...TEST
}

void AD85050_EQ_OnOff(Bool BEQ_On)
{
#if 1 //DDD...TEST
	return;
#else //DDD...TEST

	uint8_t Data = 0;
	
	//EQ
	TAS5806MD_Amp_Move_to_Control_Page();
	I2C_Interrupt_Read_Data(TAS5806MD_I2C_ADDR, TAS5806MD_DSP_MISC_REG,&Data,1);
	
	if(BEQ_On)
		Data &= ~(TAS5806MD_EQ_BYPASS_CONTROL);
	else
		Data |= TAS5806MD_EQ_BYPASS_CONTROL;
	
	I2C_Interrupt_Write_Data(TAS5806MD_I2C_ADDR, TAS5806MD_DSP_MISC_REG,&Data,1);		
#endif //DDD...TEST
}

void AD85050_DRC_OnOff(Bool BDRC_On)
{
#if 1 //DDD...TEST
	
#else //DDD...TEST
	uint8_t Data = 0;
	
	//DRC
	TAS5806MD_Amp_Move_to_Control_Page();
	I2C_Interrupt_Read_Data(TAS5806MD_I2C_ADDR, TAS5806MD_DSP_MISC_REG,&Data,1);
	
	if(BDRC_On)
		Data &= ~(TAS5806MD_DRC_BYPASS_CONTROL);
	else
		Data |= TAS5806MD_DRC_BYPASS_CONTROL;
	
	I2C_Interrupt_Write_Data(TAS5806MD_I2C_ADDR, TAS5806MD_DSP_MISC_REG,&Data,1);
#endif
}

#ifdef DRC_TOGGLE_TEST
void AD85050_DRC_On(void)
{
	uint8_t Data = 0;

	//DRC
	TAS5806MD_Amp_Move_to_Control_Page();
	I2C_Interrupt_Read_Data(TAS5806MD_I2C_ADDR, TAS5806MD_DSP_MISC_REG,&Data,1);
	Data &= ~(TAS5806MD_DRC_BYPASS_CONTROL);
	I2C_Interrupt_Write_Data(TAS5806MD_I2C_ADDR, TAS5806MD_DSP_MISC_REG,&Data,1);
	
	//AGL
	Data = 0;
	I2C_Interrupt_Write_Data(TAS5806MD_I2C_ADDR, MOVE_PAGE,&Data,1); //0x00:0x00
	Data = 0x8C;
	I2C_Interrupt_Write_Data(TAS5806MD_I2C_ADDR, MOVE_BOOK,&Data,1); //0x68:0x8c
	Data = 0x2C;
	I2C_Interrupt_Write_Data(TAS5806MD_I2C_ADDR, MOVE_PAGE,&Data,1); //0x00:0x2c
	Data = 0xC0;
	I2C_Interrupt_Write_Data(TAS5806MD_I2C_ADDR, 0x68,&Data,1); //0x68:0xc0
	Data = 0;
	I2C_Interrupt_Write_Data(TAS5806MD_I2C_ADDR, 0x69,&Data,1); //0x69:0xc0
	I2C_Interrupt_Write_Data(TAS5806MD_I2C_ADDR, 0x6A,&Data,1); //0x6A:0xc0	
	I2C_Interrupt_Write_Data(TAS5806MD_I2C_ADDR, 0x6B,&Data,1); //0x6B:0xc0
}

void AD85050_DRC_Off(void)
{
	uint8_t Data = 0;

	//DRC
	TAS5806MD_Amp_Move_to_Control_Page();
	I2C_Interrupt_Read_Data(TAS5806MD_I2C_ADDR, TAS5806MD_DSP_MISC_REG,&Data,1);
	Data |= TAS5806MD_DRC_BYPASS_CONTROL;
	I2C_Interrupt_Write_Data(TAS5806MD_I2C_ADDR, TAS5806MD_DSP_MISC_REG,&Data ,1);
	
	//AGL
	Data = 0;
	I2C_Interrupt_Write_Data(TAS5806MD_I2C_ADDR, MOVE_PAGE,&Data,1); //0x00:0x00
	Data = 0x8C;
	I2C_Interrupt_Write_Data(TAS5806MD_I2C_ADDR, MOVE_BOOK,&Data,1); //0x68:0x8c
	Data = 0x2C;
	I2C_Interrupt_Write_Data(TAS5806MD_I2C_ADDR, MOVE_PAGE,&Data,1); //0x00:0x2c
	Data = 0x40;
	I2C_Interrupt_Write_Data(TAS5806MD_I2C_ADDR, 0x68,&Data,1); //0x68:0x40
	Data = 0;
	I2C_Interrupt_Write_Data(TAS5806MD_I2C_ADDR, 0x69,&Data,1); //0x69:0xc0
	I2C_Interrupt_Write_Data(TAS5806MD_I2C_ADDR, 0x6A,&Data,1); //0x6A:0xc0	
	I2C_Interrupt_Write_Data(TAS5806MD_I2C_ADDR, 0x6B,&Data,1); //0x6B:0xc0
}
#endif

void AD85050_Amp_Volume_Register_Writing(uint8_t uVolumeLevel)
{
	uint8_t uReg_Value = 0;
	//uint8_t uArrayLevel = 0;
	int i;

	if(uVolumeLevel > 50)
		uVolumeLevel = 50;

  if(uVolumeLevel != 50)
  {
    switch(Cur_EQ_Mode)
    {
      case EQ_POP_ROCK_MODE:
      break;
      case EQ_JAZZ_MODE:
      break;
      case EQ_VOCAL_MODE:
      break;

      case EQ_NORMAL_MODE:
      case EQ_CLUB_MODE:
      //case EQ_BAP_NORMAL_MODE:
      default:
      break;
    }
  }

	uReg_Value = AD85050_Volume_Table[uVolumeLevel];
	I2C_Interrupt_Write_Data(AD85050_I2C_ADDR, AD85050_VOL_CONTROL_REG1,&uReg_Value,1);  
}

void AD85050_Dac_Volume_Set(Bool Aux_Mode) //2023-06-13_1 : Added parameter "Aux_Mode" and changed this function
{
  uint8_t Data = 0;

	Data = AD85050_DAC_GAIN_MINUS_4DB;
	I2C_Interrupt_Write_Data(AD85050_I2C_ADDR, AD85050_DAC_GAIN_REG,&Data,1);
}

#endif //AD85050_ENABLE

