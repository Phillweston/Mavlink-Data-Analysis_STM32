/*
��ֲ�ο��˲���԰  ������е�  "��ֲmavlink��stm32��ϸ�̳̣����渽������ֲ����"
����   http://www.cnblogs.com/lovechen/p/5809709.html

˵��:ʹ��stm32��ɿ�д����,Ҳ�ܶ����ɿ�����

Ӳ������: stm32����1 ����pixhawk������,ע�������ڲ����ʲ���Ҫ�����洮��1һ��
			 stm32����3���ӵ��Դ�������,�鿴��ӡ��Ϣ
ע�⣺���ӷɿ�ʱ��������վ�������ӣ�����USB����mp
�����apm/pix������ɺ�����һ��stm32

qq��1032921868
���������ɷ�������

*/

#include "delay.h"
#include "sys.h"
#include "led.h"
#include "string.h"

#include "usart.h"
#include "usart3.h"
#include "mavlink_types.h"
#include "open_tel_mavlink.h"

mavlink_system_t mavlink_system;

u32 last_mav_update_rc_time_count=0,time_100ms_count=0,time_10ms_count=0;

u8 update_1hz_finish=1,update_5hz_finish=1,update_10hz_finish=1,flag_update_arm_finish,\
	 update_600ms_finfsh=1,update_20hz_finish=1;
u8 mission_received=0; 


#define UART_TX_BUFFER_SIZE        511
#define UART_RX_BUFFER_SIZE        511

extern fifo_t uart_rx_fifo, uart_tx_fifo;
extern uint8_t uart_tx_buf[UART_TX_BUFFER_SIZE], uart_rx_buf[UART_RX_BUFFER_SIZE];

int main(void)
{
	delay_init();	    	 //��ʱ������ʼ��	  
	LED_Init();
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);	 //����NVIC�жϷ���2:2λ��ռ���ȼ���2λ��Ӧ���ȼ�
	uart_init(57600);	 	//����1��ʼ��Ϊ57600
	usart3_init(57600);
	TIM3_Int_Init(999,719);//��ʱ��10ms�ж�
	
	fifo_init(&uart_tx_fifo, uart_tx_buf, UART_TX_BUFFER_SIZE);	//��ʼ��fifo
	fifo_init(&uart_rx_fifo, uart_rx_buf, UART_RX_BUFFER_SIZE);
	
//ʵ���70�ų�Խ����,�����ɿص�������Ϣ�����ܸ�ֵӰ��
	mavlink_system.sysid = MAV_TYPE_GENERIC;
	mavlink_system.compid = MAV_COMP_ID_ALL;

	
	//����������,�����apm/pix������ɺ�����32,��֤�ɿ��յ�66����Ϣ
	//���ͳɹ�����ڴ���3������ӡ�ķɿص�����
	
	{
				tx_request_data_stream.req_stream_id=0;   //����ȫ��������
				tx_request_data_stream.req_message_rate=2;//��������,����2hz
        tx_request_data_stream.start_stop=1; 
				mavlink_send_message(MAVLINK_COMM_0, MSG_ID_REQUEST_DATA_STREAM, 0);
	}
	
	delay_ms(5000);
	
	//���ͺ���������Ϣ,����һ������д��Ĺ���
	mavlink_send_message(MAVLINK_COMM_0, MSG_ID_MISSION_COUNT, 0);//MSG_ID_MISSION_COUNT  44
//��һ���ֺ��������ڽ��ս�����,��������������������£�
	//GCS->Drone  MISSION_COUNT
	//Drone->GCS  MISSION_REQUEST(0)
	//GCS->Drone  MISSION_ITEM(0)
	//Drone->GCS  MISSION_REQUEST(1)
	//GCS->Drone  MISSION_ITEM(1)
	//Drone->GCS  MISSION_ACK
	while(1)
	{
		if(!update_1hz_finish)
			{	
				mavlink_send_message(MAVLINK_COMM_0, MSG_HEARTBEAT, 0);//����������
				
				//�ɼ���Ƿ����յ��ɿ�����	
//				u3_printf("alt:%5.2f\r\n",rx_vfr_hud.alt);
				

				update_1hz_finish=1;
				LED0=!LED0;
			}			
			
	  	update();//���ղ�����mavlink��Ϣ�ĺ������������յ���Ӧ��Ϣ���͵ĺ�����Ϣ�����ͺ���ĺ����� "open_tel_mavlink.c"��
			
			if(0) {break;} //�����и�Ī������ľ���
			
	}
	
	return 0;
	
}

 
void TIM3_IRQHandler(void)   //TIM3�ж�
{
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)  //���TIM3�����жϷ������
		{
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update  );  //���TIMx�����жϱ�־ 
			time_10ms_count++;
			
			if(time_10ms_count%5==0)
			{
				update_20hz_finish=0;
			}
			
			if(time_10ms_count%10==0)
			{
			time_100ms_count++;			
			update_10hz_finish=0;

			if(time_100ms_count%2==0)
				{
					update_5hz_finish=0;
				}
			
			if(time_100ms_count%10==0)
				{
				  update_1hz_finish=0; 
					
				}
			}
		}
}

