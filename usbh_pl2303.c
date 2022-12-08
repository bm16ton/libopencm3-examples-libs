#include "usb/usbh_pl2303.h"
#include "uusbsb_core.h"
#include "usb_hcd_int.h"

#include "usbh_stdreq.h"
#include "usb_bsp.h"
#include "usbh_ioreq.h"
#include "usbh_hcs.h"
#include "usbh_core.h"

#include "string.h"
#include <radio/fifo_c.h>
#include <utils/time_functions.h>

__ALIGN_BEGIN PL2303_Machine_TypeDef         PL2303_Machine __ALIGN_END ;
__ALIGN_BEGIN USB_Setup_TypeDef           PL2303_Setup __ALIGN_END ;

USBH_PL2303_State_TypeDef USBH_PL2303_XferParam;


#define PL2303_RX_BUFFER_SIZE	512
#define PL2303_TX_BUFFER_SIZE	512
#define PL2303_RX_FRAME_BUFFER_SIZE	64
#define PL2303_TX_FRAME_BUFFER_SIZE	64

FIFO_Structure fiforx;
FIFO_Structure fifotx;

static USBH_Status USBH_PL2303_InterfaceInit  (USB_OTG_CORE_HANDLE *pdev, void *phost);
static void USBH_PL2303_InterfaceDeInit  (USB_OTG_CORE_HANDLE *pdev, void *phost);
static USBH_Status USBH_PL2303_Handle(USB_OTG_CORE_HANDLE *pdev, void *phost);
static USBH_Status USBH_PL2303_ClassRequest(USB_OTG_CORE_HANDLE *pdev, void *phost);

USBH_Class_cb_TypeDef  USBH_PL2303_cb = {
		USBH_PL2303_InterfaceInit,
		USBH_PL2303_InterfaceDeInit,
		USBH_PL2303_ClassRequest,
		USBH_PL2303_Handle,
};

static USBH_Status USBH_PL2303_InterfaceInit ( USB_OTG_CORE_HANDLE *pdev, void *phost){
	USBH_HOST *pphost = phost;

	if(pphost->device_prop.Dev_Desc.idVendor==0x067B && pphost->device_prop.Dev_Desc.idProduct==0x2303 && pphost->device_prop.Itf_Desc[0].bInterfaceClass==0xFF && pphost->device_prop.Itf_Desc[0].bNumEndpoints==0x03){
		uint8_t i=0;
		for(i=0;i<3;i++){
			if(pphost->device_prop.Ep_Desc[0][i].bEndpointAddress & 0x80){
				if(pphost->device_prop.Ep_Desc[0][i].bmAttributes==0x03){
					PL2303_Machine.MSBulkIntInEp = (pphost->device_prop.Ep_Desc[0][i].bEndpointAddress);
					PL2303_Machine.MSBulkIntInEpSize  = pphost->device_prop.Ep_Desc[0][i].wMaxPacketSize;
				} else {
					PL2303_Machine.MSBulkInEp = (pphost->device_prop.Ep_Desc[0][i].bEndpointAddress);
					PL2303_Machine.MSBulkInEpSize  = pphost->device_prop.Ep_Desc[0][i].wMaxPacketSize;
				}
			} else {
				PL2303_Machine.MSBulkOutEp = (pphost->device_prop.Ep_Desc[0][i].bEndpointAddress);
				PL2303_Machine.MSBulkOutEpSize  = pphost->device_prop.Ep_Desc[0] [i].wMaxPacketSize;
			}
		}

		PL2303_Machine.hc_num_out = USBH_Alloc_Channel(pdev, PL2303_Machine.MSBulkOutEp);
		PL2303_Machine.hc_num_in = USBH_Alloc_Channel(pdev, PL2303_Machine.MSBulkInEp);
		PL2303_Machine.hc_num_int_in = USBH_Alloc_Channel(pdev, PL2303_Machine.MSBulkIntInEp);

		/* Open the new channels */
		USBH_Open_Channel  (pdev, PL2303_Machine.hc_num_out, pphost->device_prop.address, pphost->device_prop.speed, EP_TYPE_BULK, PL2303_Machine.MSBulkOutEpSize);
		USBH_Open_Channel  (pdev, PL2303_Machine.hc_num_in, pphost->device_prop.address, pphost->device_prop.speed, EP_TYPE_BULK, PL2303_Machine.MSBulkInEpSize);
		USBH_Open_Channel  (pdev, PL2303_Machine.hc_num_int_in, pphost->device_prop.address, pphost->device_prop.speed, EP_TYPE_INTR, PL2303_Machine.MSBulkIntInEpSize);
	} else{
		pphost->usr_cb->DeviceNotSupported();
	}
	return USBH_OK ;
}

void USBH_PL2303_InterfaceDeInit ( USB_OTG_CORE_HANDLE *pdev, void *phost){
	if ( PL2303_Machine.hc_num_out){
		USB_OTG_HC_Halt(pdev, PL2303_Machine.hc_num_out);
		USBH_Free_Channel  (pdev, PL2303_Machine.hc_num_out);
		PL2303_Machine.hc_num_out = 0;     /* Reset the Channel as Free */
	}

	if ( PL2303_Machine.hc_num_in){
		USB_OTG_HC_Halt(pdev, PL2303_Machine.hc_num_in);
		USBH_Free_Channel  (pdev, PL2303_Machine.hc_num_in);
		PL2303_Machine.hc_num_in = 0;     /* Reset the Channel as Free */
	}

	if ( PL2303_Machine.hc_num_int_in){
		USB_OTG_HC_Halt(pdev, PL2303_Machine.hc_num_int_in);
		USBH_Free_Channel  (pdev, PL2303_Machine.hc_num_int_in);
		PL2303_Machine.hc_num_int_in = 0;     /* Reset the Channel as Free */
	}
}

static USBH_Status USBH_PL2303_ClassRequest(USB_OTG_CORE_HANDLE *pdev, void *phost){
	USBH_Status status = USBH_OK ;
	USBH_PL2303_XferParam.PL2303State = USBH_PL2303_INIT_STATE;
	return status;
}

USBH_Status pl2303_vendor_read(USB_OTG_CORE_HANDLE *pdev , USBH_HOST *phost, uint16_t value, uint16_t index, uint8_t *buf, uint8_t length){
	phost->Control.setup.b.bmRequestType = USB_D2H | USB_REQ_TYPE_VENDOR | USB_REQ_RECIPIENT_DEVICE;
	phost->Control.setup.b.bRequest = 0x01;
	phost->Control.setup.b.wValue.w = value;
	phost->Control.setup.b.wIndex.w = index;
	phost->Control.setup.b.wLength.w = length;
	return USBH_CtlReq(pdev, phost, buf , length );
}

USBH_Status pl2303_vendor_write(USB_OTG_CORE_HANDLE *pdev , USBH_HOST *phost, uint16_t value, uint16_t index){
	phost->Control.setup.b.bmRequestType = USB_H2D | USB_REQ_TYPE_VENDOR | USB_REQ_RECIPIENT_DEVICE;
	phost->Control.setup.b.bRequest = 0x01;
	phost->Control.setup.b.wValue.w = value;
	phost->Control.setup.b.wIndex.w = index;
	phost->Control.setup.b.wLength.w = 0;
	return USBH_CtlReq(pdev, phost, 0 , 0 );
}


uint32_t pl2303_get_baudrate(uint32_t baud){
	const int baud_sup[] = { 75, 150, 300, 600, 1200, 1800, 2400, 3600, 4800, 7200, 9600, 14400, 19200, 28800, 38400, 57600, 115200, 230400, 460800, 500000, 614400, 921600, 1228800, 2457600, 3000000, 6000000 };
	uint8_t i=0;
	while(baud_sup[i]<baud){
		i++;
	}
	return baud_sup[i];
}


USBH_Status pl2303_set_serial(USB_OTG_CORE_HANDLE *pdev , USBH_HOST *phost, uint32_t baudrate, uint8_t databits, uint8_t stopbits, uint8_t parity){
	static uint8_t set_serial_data[7];
	baudrate=pl2303_get_baudrate(baudrate);
	if(databits<5){
		databits=5;
	}
	if(databits>8){
		databits=8;
	}
	set_serial_data[0]=baudrate & 0xff;
	set_serial_data[1]=(baudrate >> 8) & 0xff;
	set_serial_data[2]=(baudrate >> 16) & 0xff;
	set_serial_data[3]=(baudrate >> 24) & 0xff;
	set_serial_data[4]=stopbits;
	set_serial_data[5]=parity;
	set_serial_data[6]=databits;

	phost->Control.setup.b.bmRequestType = USB_H2D | USB_REQ_TYPE_CLASS | USB_REQ_RECIPIENT_INTERFACE;
	phost->Control.setup.b.bRequest = SET_LINE_REQUEST;
	phost->Control.setup.b.wValue.w = 0;
	phost->Control.setup.b.wIndex.w = 0;
	phost->Control.setup.b.wLength.w = 7;
	return USBH_CtlReq(pdev, phost, set_serial_data , 7 );
}

uint8_t USBH_PL2303_SendAvailable(){
	if(!fifotx.Initiated){
		return 0;
	}
	return FIFO_Ocupied(&fifotx);
}

uint16_t USBH_PL2303_SendData(uint8_t *buffer, uint16_t len){
	if(!fifotx.Initiated){
		return 0;
	}
	return FIFO_In(&fifotx, buffer, len);
}

uint16_t USBH_PL2303_GetData(uint8_t *buffer, uint16_t len){
	if(!fiforx.Initiated){
		return 0;
	}
	uint16_t ocupied=FIFO_Ocupied(&fiforx);
	if(len>ocupied){
		len=ocupied;
	}
	if(len==0){
		return len;
	}
	FIFO_Out(&fiforx, buffer, len);
	return len;
}

uint16_t USBH_PL2303_GetDataCount(){
	if(!fiforx.Initiated){
		return 0;
	}
	return FIFO_Ocupied(&fiforx);
}

static USBH_Status USBH_PL2303_Handle(USB_OTG_CORE_HANDLE *pdev , void   *phost){
//	USBH_HOST *pphost = phost;
	USBH_Status status = USBH_BUSY;
	URB_STATE URB_Status = URB_IDLE;
	static uint8_t pl2303_rx_frame_buffer[PL2303_RX_FRAME_BUFFER_SIZE];
	static uint8_t pl2303_tx_frame_buffer[PL2303_TX_FRAME_BUFFER_SIZE];
	static uint8_t pl2303_rx_buffer[PL2303_RX_BUFFER_SIZE*2];
	static uint8_t pl2303_tx_buffer[PL2303_TX_BUFFER_SIZE*2];
	static uint16_t framenumber_old=0;
	static uint32_t timestamp_old=0;
	uint16_t framenumber=HCD_GetCurrentFrame(pdev);
	uint32_t timestamp=GetLocalTime();
	if(HCD_IsDeviceConnected(pdev)){
		switch(USBH_PL2303_XferParam.PL2303State){
		case USBH_PL2303_INIT_STATE:
			FIFO_Init(&fiforx, pl2303_rx_buffer, PL2303_RX_BUFFER_SIZE);
			FIFO_Init(&fifotx, pl2303_tx_buffer, PL2303_TX_BUFFER_SIZE);
			status=pl2303_vendor_read(pdev, phost, 0x9494, 0, pl2303_tx_buffer, 1);
			if(status == USBH_OK && pl2303_tx_buffer[0]==0){
				USBH_PL2303_XferParam.PL2303State = USBH_PL2303_INIT_STATEA;
			} else if(status == USBH_NOT_SUPPORTED ){
				USBH_PL2303_XferParam.PL2303State = USBH_PL2303_ERROR;
			}
			break;
		case USBH_PL2303_INIT_STATEA:
			status=pl2303_vendor_write(pdev, phost, 0x0606, 0x00AA);
			if(status == USBH_OK){
				USBH_PL2303_XferParam.PL2303State = USBH_PL2303_INIT_STATEB;
			} else if(status == USBH_NOT_SUPPORTED ){
				USBH_PL2303_XferParam.PL2303State = USBH_PL2303_ERROR;
			}
			break;
		case USBH_PL2303_INIT_STATEB:
			status=pl2303_vendor_read(pdev, phost, 0x8686, 0, pl2303_tx_buffer, 1);
			if(status == USBH_OK  && pl2303_tx_buffer[0]==0xAA){
				USBH_PL2303_XferParam.PL2303State = USBH_PL2303_INIT_STATEC;
			} else if(status == USBH_NOT_SUPPORTED ){
				USBH_PL2303_XferParam.PL2303State = USBH_PL2303_ERROR;
			}
			break;
		case USBH_PL2303_INIT_STATEC:
			status=pl2303_vendor_write(pdev, phost, 0x0606, 0x0000);
			if(status == USBH_OK){
				USBH_PL2303_XferParam.PL2303State = USBH_PL2303_INIT_STATE0;
			} else if(status == USBH_NOT_SUPPORTED ){
				USBH_PL2303_XferParam.PL2303State = USBH_PL2303_ERROR;
			}
			break;
		case USBH_PL2303_INIT_STATE0:
			status=pl2303_vendor_read(pdev, phost, 0x8484, 0, pl2303_tx_buffer, 1);
			if(status == USBH_OK && pl2303_tx_buffer[0]==0x02){
				USBH_PL2303_XferParam.PL2303State = USBH_PL2303_INIT_STATE1;
			} else if(status == USBH_NOT_SUPPORTED ){
				USBH_PL2303_XferParam.PL2303State = USBH_PL2303_ERROR;
			}
			break;
		case USBH_PL2303_INIT_STATE1:
			status=pl2303_vendor_write(pdev, phost, 0x0404, 0);
			if(status == USBH_OK ){
				USBH_PL2303_XferParam.PL2303State = USBH_PL2303_INIT_STATE2;
			} else if(status == USBH_NOT_SUPPORTED ){
				USBH_PL2303_XferParam.PL2303State = USBH_PL2303_ERROR;
			}
			break;
		case USBH_PL2303_INIT_STATE2:
			status=pl2303_vendor_read(pdev, phost, 0x8484, 0, pl2303_tx_buffer, 1);
			if(status == USBH_OK && pl2303_tx_buffer[0]==0x02){
				USBH_PL2303_XferParam.PL2303State = USBH_PL2303_INIT_STATE3;
			} else if(status == USBH_NOT_SUPPORTED ){
				USBH_PL2303_XferParam.PL2303State = USBH_PL2303_ERROR;
			}
			break;
		case USBH_PL2303_INIT_STATE3:
			status=pl2303_vendor_read(pdev, phost, 0x8383, 0, pl2303_tx_buffer, 1);
			if(status == USBH_OK){
				USBH_PL2303_XferParam.PL2303State = USBH_PL2303_INIT_STATE4;
			} else if(status == USBH_NOT_SUPPORTED ){
				USBH_PL2303_XferParam.PL2303State = USBH_PL2303_ERROR;
			}
			break;
		case USBH_PL2303_INIT_STATE4:
			status=pl2303_vendor_read(pdev, phost, 0x8484, 0, pl2303_tx_buffer, 1);
			if(status == USBH_OK && pl2303_tx_buffer[0]==0x02){
				USBH_PL2303_XferParam.PL2303State = USBH_PL2303_INIT_STATE5;
			} else if(status == USBH_NOT_SUPPORTED ){
				USBH_PL2303_XferParam.PL2303State = USBH_PL2303_ERROR;
			}
			break;
		case USBH_PL2303_INIT_STATE5:
			status=pl2303_vendor_write(pdev, phost, 0x0404, 1);
			if(status == USBH_OK ){
				USBH_PL2303_XferParam.PL2303State = USBH_PL2303_INIT_STATE6;
			} else if(status == USBH_NOT_SUPPORTED ){
				USBH_PL2303_XferParam.PL2303State = USBH_PL2303_ERROR;
			}
			break;
		case USBH_PL2303_INIT_STATE6:
			status=pl2303_vendor_read(pdev, phost, 0x8484, 0, pl2303_tx_buffer, 1);
			if(status == USBH_OK && pl2303_tx_buffer[0]==0x02){
				USBH_PL2303_XferParam.PL2303State = USBH_PL2303_INIT_STATE7;
			} else if(status == USBH_NOT_SUPPORTED ){
				USBH_PL2303_XferParam.PL2303State = USBH_PL2303_ERROR;
			}
			break;
		case USBH_PL2303_INIT_STATE7:
			status=pl2303_vendor_read(pdev, phost, 0x8383, 0, pl2303_tx_buffer, 1);
			if(status == USBH_OK){
				USBH_PL2303_XferParam.PL2303State = USBH_PL2303_INIT_STATE8;
			} else if(status == USBH_NOT_SUPPORTED ){
				USBH_PL2303_XferParam.PL2303State = USBH_PL2303_ERROR;
			}
			break;
		case USBH_PL2303_INIT_STATE8:
			status=pl2303_vendor_write(pdev, phost, 0, 1);
			if(status == USBH_OK ){
				USBH_PL2303_XferParam.PL2303State = USBH_PL2303_INIT_STATE9;
			} else if(status == USBH_NOT_SUPPORTED ){
				USBH_PL2303_XferParam.PL2303State = USBH_PL2303_ERROR;
			}
			break;
		case USBH_PL2303_INIT_STATE9:
			status=pl2303_vendor_write(pdev, phost, 1, 0);
			if(status == USBH_OK ){
				USBH_PL2303_XferParam.PL2303State = USBH_PL2303_INIT_STATE10;
			} else if(status == USBH_NOT_SUPPORTED ){
				USBH_PL2303_XferParam.PL2303State = USBH_PL2303_ERROR;
			}
			break;
		case USBH_PL2303_INIT_STATE10:
			status=pl2303_vendor_write(pdev, phost, 2, 0x44);
			if(status == USBH_OK ){
				USBH_PL2303_XferParam.PL2303State = USBH_PL2303_INIT_STATE11;
			} else if(status == USBH_NOT_SUPPORTED ){
				USBH_PL2303_XferParam.PL2303State = USBH_PL2303_ERROR;
			}
			break;
		case USBH_PL2303_INIT_STATE11:
			status=pl2303_set_serial(pdev, phost, 38400, 8, PL2303_STOPBITS_1, PL2303_PARITY_NONE);
			if(status == USBH_OK ){
				USBH_PL2303_XferParam.PL2303State = USBH_PL2303_INIT_OK;
				USBH_PL2303_SendData((uint8_t*)"TPP-1400 Connected!\r\n", 21);
			} else if(status == USBH_NOT_SUPPORTED ){
				USBH_PL2303_XferParam.PL2303State = USBH_PL2303_ERROR;
			}
			USBH_BulkReceiveData(pdev, pl2303_rx_frame_buffer, PL2303_Machine.MSBulkInEpSize, PL2303_Machine.hc_num_in);
			break;

		case USBH_PL2303_INIT_OK:
			if(FIFO_Ocupied(&fifotx)){
				uint16_t len=FIFO_Out(&fifotx, pl2303_tx_frame_buffer, sizeof(pl2303_tx_frame_buffer));
				USBH_BulkSendData(pdev, pl2303_tx_frame_buffer, len, PL2303_Machine.hc_num_out);
				status=USBH_OK;
				break;
			}
			URB_Status = HCD_GetURB_State(pdev , PL2303_Machine.hc_num_in);
			if(URB_Status == URB_DONE){
				uint8_t rxcount=HCD_GetXferCnt(pdev, PL2303_Machine.hc_num_in);
				FIFO_In(&fiforx, pl2303_rx_frame_buffer, rxcount);
				USBH_BulkReceiveData(pdev, pl2303_rx_frame_buffer, PL2303_Machine.MSBulkInEpSize, PL2303_Machine.hc_num_in);
			}
			if((pdev->host.HC_Status[PL2303_Machine.hc_num_in] == HC_IDLE) && ((framenumber-framenumber_old)>100)){
				USBH_BulkReceiveData(pdev, pl2303_rx_frame_buffer, PL2303_Machine.MSBulkInEpSize, PL2303_Machine.hc_num_in);
			}
			if((pdev->host.HC_Status[PL2303_Machine.hc_num_in] == HC_IDLE) && ((timestamp-timestamp_old)>20000)){
//TODO				RADIO_EVENT_Cmd(EVENT_SWITCH_CHANGE, ENABLE);
			}
			if((pdev->host.HC_Status[PL2303_Machine.hc_num_in] != HC_XFRC) && (pdev->host.HC_Status[PL2303_Machine.hc_num_in] != HC_NAK) && (pdev->host.HC_Status[PL2303_Machine.hc_num_in] != HC_IDLE) && (framenumber-framenumber_old)>100){
				USBH_BulkReceiveData(pdev, pl2303_rx_frame_buffer, PL2303_Machine.MSBulkInEpSize, PL2303_Machine.hc_num_in);
			}
			if(pdev->host.HC_Status[PL2303_Machine.hc_num_in] == HC_NAK){
				pdev->host.HC_Status[PL2303_Machine.hc_num_in] = HC_IDLE;
				framenumber_old=framenumber;
				timestamp_old=timestamp;
			}

			status=USBH_OK;
			break;
		default:
			break;
		}
	}
	return status;
}


