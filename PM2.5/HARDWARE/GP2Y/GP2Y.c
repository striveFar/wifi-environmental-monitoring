#include "GP2Y.h"
#include "stm32f10x.h"
#include "delay.h"
#include "usart.h"
#include "adc.h"
#include "led.h"
	u32 AD_PM;
void GP2Yinit(void)
{ 
//定义变量
    ADC_InitTypeDef A_InitStructure;
    GPIO_InitTypeDef G_InitStructure;	//PA0
	
    GPIO_InitTypeDef Gpio_InitStructure;//PB1
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA |RCC_APB2Periph_GPIOB|RCC_APB2Periph_ADC1 , ENABLE );   //使能ADC2通道时钟
     
    RCC_ADCCLKConfig(RCC_PCLK2_Div6);   //设置ADC分频因子6 72M/6=12,ADC最大时间不能超过14M
 
    //PA0 作为模拟通道输入引脚                        
    G_InitStructure.GPIO_Pin = GPIO_Pin_6;
    G_InitStructure.GPIO_Mode = GPIO_Mode_AIN; //模拟输入引脚
    GPIO_Init(GPIOA, &G_InitStructure);
   
    ADC_DeInit(ADC1);  //复位ADC1,将外设 ADC1 的全部寄存器重设为缺省值
   
    A_InitStructure.ADC_Mode = ADC_Mode_Independent; //ADC工作模式:ADC1和ADC2工作在独立模式
    A_InitStructure.ADC_ScanConvMode = DISABLE; //模数转换工作在单通道模式
    A_InitStructure.ADC_ContinuousConvMode = ENABLE; //模数转换工作在单次转换模式
    A_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None; //转换由软件而不是外部触发启动
    A_InitStructure.ADC_DataAlign = ADC_DataAlign_Right; //ADC数据右对齐
    A_InitStructure.ADC_NbrOfChannel = 1; //顺序进行规则转换的ADC通道的数目
    ADC_Init(ADC1, &A_InitStructure); //根据ADC_InitStruct中指定的参数初始化外设ADCx的寄存器  
   
   
    ADC_Cmd(ADC1, ENABLE);         //使能指定的ADC1
		
    ADC_ResetCalibration(ADC1); //使能复位校准 
    while(ADC_GetResetCalibrationStatus(ADC1)); //等待复位校准结束
    ADC_StartCalibration(ADC1);  //开启AD校准 
    while(ADC_GetCalibrationStatus(ADC1));  //等待校准结束
		
    Gpio_InitStructure.GPIO_Pin = GPIO_Pin_1;	                 
    Gpio_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;//推挽输出  
    Gpio_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;           
    GPIO_Init(GPIOB, &Gpio_InitStructure);
    GP2Y_High;
}

//获得ADC值
//ch:通道值 0~3
u16 Get_Adc(u8 ch)   
{
  	//设置指定ADC的规则组通道，一个序列，采样时间
	ADC_RegularChannelConfig(ADC1, ch, 1, ADC_SampleTime_239Cycles5 );	//ADC1,ADC通道,采样时间为239.5周期	  			    
  
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);		//使能指定的ADC1的软件转换启动功能	
	 
	while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC ));//等待转换结束

	return ADC_GetConversionValue(ADC1);	//返回最近一次ADC1规则组的转换结果
}

u16 Get_Adc_Average(u8 ch,u8 times)
{
	u32 temp_val=0;
	u8 t;
	for(t=0;t<times;t++)
	{
		temp_val+=Get_Adc(ch);
		delay_ms(5);
	}
	return temp_val/times;
} 	 


float pma,values;
void GetGP2Y(void)
{

	GP2Y_Low;//打开红外二极管
	delay_us(280); //延时280us//  
	AD_PM = Get_Adc(ADC_Channel_6);	//PA0, 采样, 读取AD值//  
	delay_us(40);//19 + 21 +280 =320
	GP2Y_High;//关闭红外二极管
	delay_us(9680);
	values=AD_PM*3.3/4096;
    /*如果乘以1000单位换成ug/m3，这里没有乘*/
	pma = 0.17*AD_PM-0.1; //电压-灰尘转换
}

