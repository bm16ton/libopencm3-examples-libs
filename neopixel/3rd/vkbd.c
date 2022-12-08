#include <stdlib.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/scb.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/stm32/desig.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/cm3/systick.h>
#include <libopencm3/stm32/can.h>

#include <atom.h>
#include <atomsem.h>
#include <atomqueue.h>
#include <atomtimer.h>
#include <string.h>
#include "ws2812b.h"

#include "hw.h"

#define DEBUG 1

#define C_OFF     0
#define C_WHITE   1
#define C_RED     2
#define C_GREEN   3
#define C_BLUE    4
#define C_YELLOW  5
#define C_CYAN    6
#define C_MAGENTA 7

static uint8_t idle_stack[256];
static uint8_t master_thread_stack[512];
static ATOM_TCB master_thread_tcb;

static uint8_t scan_thread_stack[512];
static ATOM_TCB scan_thread_tcb;

static ATOM_QUEUE buttons_q;
static uint8_t buttons_storage[16];

static ATOM_QUEUE uart1_tx;
static uint8_t uart1_tx_storage[1024];

static ATOM_QUEUE uart1_rx;
static uint8_t uart1_rx_storage[64];

void _fault(int, int, const char*);
int u_write(int file, uint8_t *ptr, int len);
int s_write(int file, char *ptr, int len);
void xcout(unsigned char c);
void x2cout(uint16_t c);

uint32_t colors[16] = {
  0,
  0xffffff,
  0xff0000,
  0x00Ff00,
  0x0000fF,
  0xffFf00,
  0x00Ff99,
  0xFf00ff,

  0xffffff,
  0xff0000,
  0x00Ff00,
  0x0000fF,
  0xffFf00,
  0x00Ff99,
  0xFf00ff,
  0};


uint8_t panel_phase=0;
uint8_t panel_btns[8];
uint8_t panel_btev=0;
uint32_t leds[8]={
  0x110011,
  0x001100,
  0x110011,
  0x001100,
  0x110011,
  0x001100,
  0x110011,
  0x001100
};

#define fault(code) _fault(code,__LINE__,__FUNCTION__)
void _fault(__unused int code, __unused int line, __unused const char* function){
    cm_mask_interrupts(true);
    while(1){
    }
};


void xcout(unsigned char c){
    static char set[]="0123456789ABCDEF";
    char s[2];
    s[0]=set[(c>>4)&0x0f];
    s[1]=set[c&0x0f];
    s_write(1,s,2);
}

void x2cout(uint16_t c){
    static char set[]="0123456789ABCDEF";
    char s[4];
    s[0]=set[(c>>12)&0x0f];
    s[1]=set[(c>>8)&0x0f];
    s[2]=set[(c>>4)&0x0f];
    s[3]=set[c&0x0f];
    s_write(1,s,4);
}

void usart1_isr(void) {
    static uint8_t data = 'A';
    atomIntEnter();

    if (((USART_CR1(USART1) & USART_CR1_RXNEIE) != 0) &&
            ((USART_SR(USART1) & USART_SR_RXNE) != 0)) {
        data = usart_recv(USART1);
        atomQueuePut(&uart1_rx, -1, (uint8_t*) &data);
    }

    /* Check if we were called because of TXE. */
    if (((USART_CR1(USART1) & USART_CR1_TXEIE) != 0) &&
            ((USART_SR(USART1) & USART_SR_TXE) != 0)) {
        uint8_t status = atomQueueGet(&uart1_tx, 0, &data);
        if(status == ATOM_OK){
            usart_send_blocking(USART1, data);
        }else{
            USART_CR1(USART1) &= ~USART_CR1_TXEIE;
        }
    }
    atomIntExit(0);
}


static void scan_thread(uint32_t args __maybe_unused) {
  s_write(1,"SCAN\r\n",8);
  uint8_t evt;
  while(1){
    uint8_t status = atomQueueGet(&buttons_q, SYSTEM_TICKS_PER_SEC, (void*)&evt);
    if(status==ATOM_OK){
      if(evt & 0x80){
        s_write(1,"P",1);
      }else{
        s_write(1,"R",1);
      }
      xcout(evt & 0x7F);
      int8_t cr=-1;
      uint8_t evbuf[2];
      evbuf[0]=1;
      evbuf[1]=evt;
      if ((cr=can_transmit(CAN1,
              0x0f313301,/* (EX/ST)ID: CAN ID */
              true, /* IDE: CAN ID extended? */
              false, /* RTR: Request transmit? */
              2,     /* DLC: Data length */
              evbuf)) == -1){
        s_write(1,"CAN_ERR\r\n",9);
      }else{
        xcout((uint8_t)cr);
        s_write(1,"\r\n",2);
      }
    }
    //atomTimerDelay(SYSTEM_TICKS_PER_SEC>>6);
  }
}

static int8_t scan(uint8_t c) {
  if(c>='0' && c<='9') return c-'0';
  if(c>='a' && c<='f') return c-'a'+10;
  if(c>='A' && c<='F') return c-'A'+10;
  return -1;
}

static void master_thread(uint32_t args __maybe_unused) {
  s_write(1,"MASTER\r\n",8);
  uint8_t recv[16];
  uint8_t rlen=0;
  while(1){
    uint8_t status = atomQueueGet(&uart1_rx, SYSTEM_TICKS_PER_SEC>>2, (void*)&recv[rlen]);
    if(status == ATOM_OK){
      if(recv[rlen]=='\n' || recv[rlen]=='\r' || rlen==15){
        if(recv[0]=='C' && rlen==3){
          int8_t chan=scan(recv[1]);
          if(chan>=0 && chan<8){
            if(recv[2]=='k') leds[chan]=colors[C_OFF]; else
            if(recv[2]=='w') leds[chan]=colors[C_WHITE]; else
            if(recv[2]=='m') leds[chan]=colors[C_MAGENTA]; else
            if(recv[2]=='r') leds[chan]=colors[C_RED]; else
            if(recv[2]=='g') leds[chan]=colors[C_GREEN]; else
            if(recv[2]=='b') leds[chan]=colors[C_BLUE]; else
            if(recv[2]=='c') leds[chan]=colors[C_CYAN]; else
            if(recv[2]=='m') leds[chan]=colors[C_MAGENTA]; else
            if(recv[2]=='y') leds[chan]=colors[C_YELLOW]; else {
              int8_t p=scan(recv[2]);
              if(p>=0)
                leds[chan]=colors[p];
            }
          }
        }else if(recv[0]=='C' && rlen==5){
          int8_t chan=scan(recv[1]);
          if(chan>=0 && chan<8){
            int8_t red=scan(recv[2]);
            int8_t green=scan(recv[3]);
            int8_t blue=scan(recv[4]);
            uint32_t rgb=(red<<16)|(red<<20)|(green<<12)|(green<<8)|(blue<<4)|blue;
            leds[chan]=rgb;
          }
/*        }else if(recv[0]=='M' && rlen==5){
          int8_t col=scan(recv[1]);
          int8_t idx=scan(recv[2]);
          int8_t hi=scan(recv[3]);
          int8_t lo=scan(recv[4]);
          xcout(col);
          xcout(idx);
          if(col>=0 && col<3 && idx>=0 && idx<=15 && hi>=0 && lo>=0){
            xcout((hi<<4)|lo);
            s_write(1,"(",1);
            xcout(colormap[col][idx]);
            colormap[col][idx]=(hi<<4)|lo;
            s_write(1,"~OK\r\n",5);
          }else
            s_write(1,"~ER\r\n",5);*/
        }else{
          s_write(1,"~",1);
          xcout(rlen);

          if(recv[0]=='q' || recv[0]=='a'){
            uint16_t l=(leds[0]&0xf0000) >> 16;
            if(recv[0]=='q') if(l<0xf) l++;
            if(recv[0]=='a') if(l>0x0) l--;
            x2cout(leds[0]);
            leds[0]=(leds[0]&0xffff)|(l<<16)|(l<<20);
            s_write(1," ",1);
            x2cout(leds[0]);
          }

          if(recv[0]=='w' || recv[0]=='s'){
            uint32_t l=(leds[0]&0xf00) >> 8;
            if(recv[0]=='w') if(l<0xf) l++;
            if(recv[0]=='s') if(l>0x0) l--;
            x2cout(leds[0]);
            leds[0]=(leds[0]&0xff00ff)|(l<<8)|(l<<12);
            s_write(1," ",1);
            x2cout(leds[0]);
          }


          if(recv[0]=='e' || recv[0]=='d'){
            uint16_t l=(leds[0]&0xf);
            if(recv[0]=='e') if(l<0xf) l++;
            if(recv[0]=='d') if(l>0x0) l--;
            x2cout(leds[0]);
            leds[0]=(leds[0]&0xffff00)|l|(l<<4);
            s_write(1," ",1);
            x2cout(leds[0]);
          }
          if(recv[0]=='z') leds[0]=0xf00;
          if(recv[0]=='x') leds[0]=0x0f0;
          if(recv[0]=='c') leds[0]=0x00f;
          s_write(1,"\r\n",2);
        }
        rlen=0;
      }else
        rlen++;
    }
    ws2812b_send_pixels(leds,8);
    gpio_toggle(GPIOC, GPIO13);
    //atomTimerDelay();
  }
}

static void tim2_setup(void) {
  /* Enable TIM2 clock. */
  rcc_periph_clock_enable(RCC_TIM2);

  /* Enable TIM2 interrupt. */
  nvic_enable_irq(NVIC_TIM2_IRQ);

  /* Reset TIM2 peripheral. */
  timer_reset(TIM2);

  /* Timer global mode:
   * - No divider
   * - Alignment edge
   * - Direction up
   */
  timer_set_mode(TIM2, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE, TIM_CR1_DIR_DOWN);

  /* Reset prescaler value. */
  timer_set_prescaler(TIM2, 2);

  /* Enable preload. */
  timer_enable_preload(TIM2);

  /* Continous mode. */
  timer_continuous_mode(TIM2);

  /* Period (36kHz). */
  timer_set_period(TIM2, 65535);

  /* Disable outputs. */
  timer_disable_oc_output(TIM2, TIM_OC1);
  timer_disable_oc_output(TIM2, TIM_OC2);
  timer_disable_oc_output(TIM2, TIM_OC3);
  timer_disable_oc_output(TIM2, TIM_OC4);

  /* ARR reload enable. */
  timer_disable_preload(TIM2);

  /* Counter enable. */
  timer_enable_counter(TIM2);

  /* Enable commutation interrupt. */
  //timer_enable_irq(TIM2, TIM_DIER_CC1IE);
  /*
     if (atomSemCreate (&ibusy, 0) != ATOM_OK) 
     fault(3);
     */
  TIM_SR(TIM2) &= ~TIM_SR_UIF;
  //    timer_set_counter(TIM2, t);
  TIM_DIER(TIM2) |= TIM_DIER_UIE;
}

void tim2_isr(void) {
  atomIntEnter();
  if (timer_get_flag(TIM2, TIM_SR_UIF)) {
    TIM_SR(TIM2) &= ~TIM_SR_UIF;

    uint8_t port=gpio_port_read(GPIOB);
    uint8_t pre_phase=panel_phase;
    switch (panel_phase) {
      case 0: gpio_clear(GPIOB,GPIO12); break;
      case 1: gpio_clear(GPIOB,GPIO13); break;
      case 2: gpio_clear(GPIOB,GPIO14); break;
      case 3: gpio_clear(GPIOA,GPIO8); break;
    }
    panel_phase++;
    if(panel_phase>3) panel_phase=0;

    switch (panel_phase) {
      case 0: gpio_set(GPIOB,GPIO12); break;
      case 1: gpio_set(GPIOB,GPIO13); break;
      case 2: gpio_set(GPIOB,GPIO14); break;
      case 3: gpio_set(GPIOA,GPIO8); break;
    }

#define BTN_DELAY 4
    uint8_t btn_active=((port & GPIO3)?0:2)|((port & GPIO4)?0:1);
    for(int btnbit=0;btnbit<2;btnbit++){
      uint8_t bn = (pre_phase<<1)+btnbit;
      if(btn_active & (1<<btnbit)){
        if(panel_btns[bn]<BTN_DELAY){
          panel_btns[bn]++;
          if(panel_btns[bn]==BTN_DELAY && ((panel_btev&(1<<bn))==0)){
            uint8_t data=0x80 | bn;
            if(ATOM_OK==atomQueuePut(&buttons_q, -1, (uint8_t*) &data))
              panel_btev|=1<<bn;
            else
              panel_btns[bn]--;
          }
        }
      }else{
        if(panel_btns[bn]>0){
          panel_btns[bn]--;
          if(panel_btns[bn]==0 && ((panel_btev&(1<<bn))!=0)){
            if(ATOM_OK==atomQueuePut(&buttons_q, -1, (uint8_t*) &bn))
              panel_btev&=~(1<<bn);
            else
              panel_btns[bn]++;
          }
        }
      }
    }
  }
  atomIntExit(0);
}

static void can_setup(void) {
  /* Enable peripheral clocks. */
  rcc_periph_clock_enable(RCC_AFIO);
  rcc_periph_clock_enable(RCC_GPIOA);
  rcc_periph_clock_enable(RCC_CAN);
  AFIO_MAPR |= AFIO_MAPR_CAN1_REMAP_PORTB;
//  AFIO_MAPR &= ~AFIO_MAPR_CAN1_REMAP_PORTB;

  /* Configure CAN pin: RX (input pull-up). */
  gpio_set_mode(GPIOB, GPIO_MODE_INPUT,
      GPIO_CNF_INPUT_PULL_UPDOWN, GPIO_CAN_PB_RX);
  gpio_set(GPIOB, GPIO_CAN_PB_RX);

  /* Configure CAN pin: TX. */
  gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_50_MHZ,
      GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO_CAN_PB_TX);

  /* NVIC setup. */
  nvic_enable_irq(NVIC_USB_LP_CAN_RX0_IRQ);
  nvic_set_priority(NVIC_USB_LP_CAN_RX0_IRQ, 1);

  /* Reset CAN. */
  can_reset(CAN1);

  /*
   * 75 - 20
   * 150 - 10
   * 30 - 50
   * 15 - 100
   * 12 - 250
   * 6 - 500
   * 3 - 1000
   */

  /* CAN cell init. */
  if (can_init(CAN1,
        false,           /* TTCM: Time triggered comm mode? */
        true,            /* ABOM: Automatic bus-off management? */
        false,           /* AWUM: Automatic wakeup mode? */
        false,           /* NART: No automatic retransmission? */
        false,           /* RFLM: Receive FIFO locked mode? */
        false,           /* TXFP: Transmit FIFO priority? */
        CAN_BTR_SJW_1TQ,
        CAN_BTR_TS1_4TQ,
        CAN_BTR_TS2_3TQ,
        6,/* BRP+1: Baud rate prescaler */
        false,
        false))
  {
    gpio_set(GPIOC, GPIO13);		/* LED0*/
    /* Die because we failed to initialize. */
    while (1)
      __asm__("nop");
  }

  can_filter_id_mask_32bit_init(
      CAN1,
      0,     /* Filter ID */
      0x0f313300, /* CAN ID */
      0x00000000, /* CAN ID mask */
      0,     /* FIFO assignment (here: FIFO0) */
      true); /* Enable the filter. */

  /* Enable CAN RX interrupt. */
  can_enable_irq(CAN1, CAN_IER_FMPIE0);
}

void usb_lp_can_rx0_isr(void) {
    atomIntEnter();
  uint32_t id, fmi;
  bool ext, rtr;
  uint8_t length, data[8];

  can_receive(CAN1, 0, false, &id, &ext, &rtr, &fmi, &length, data);
  if(id==0x0f313300){
    s_write(1,"CAN_RECV ",9);
    xcout(length);
    xcout(data[0]);
    s_write(1,"\r\n",2);
    if(length==2 && (data[0]&0x0f)==6)
      leds[(data[0] >> 4)]=colors[data[1] & 0x0f];
    else if(length==4 && (data[0]&0xf)==6)
      leds[(data[0] >> 4)]=(data[1] << 16) | (data[2] << 8) | (data[3]);
    else if(length==4 && (data[0]&0xf)==4){
      colors[(data[0] >> 4)]=(data[1] << 16) | (data[2] << 8) | (data[3]);
      xcout(data[0]>>4);
      xcout(colors[data[0]>>4] >> 16);
      xcout(colors[data[0]>>4] >> 8);
      xcout(colors[data[0]>>4]);
    } else if(length==5 && data[0]==2){
      leds[0]=colors[data[1] >> 4];
      leds[1]=colors[data[1] & 0x0f];
      leds[2]=colors[data[2] >> 4];
      leds[3]=colors[data[2] & 0x0f];
      leds[4]=colors[data[3] >> 4];
      leds[5]=colors[data[3] & 0x0f];
      leds[6]=colors[data[4] >> 4];
      leds[7]=colors[data[4] & 0x0f];
    }else{
    s_write(1,"?\r\n",3);
    }
  }


  can_fifo_release(CAN1, 0);
  atomIntExit(0);
}

int main(void) {
  //rcc_clock_setup_in_hsi_out_48mhz();
  rcc_clock_setup_in_hse_8mhz_out_24mhz();
  //rcc_clock_setup_in_hsi_out_64mhz();

  rcc_periph_clock_enable(RCC_GPIOA);
  rcc_periph_clock_enable(RCC_GPIOB);
  rcc_periph_clock_enable(RCC_GPIOC);
  rcc_periph_clock_enable(RCC_AFIO);
  rcc_periph_clock_enable(RCC_USART1);
  //rcc_periph_clock_enable(RCC_SPI2);

//  scb_set_priority_grouping(uint32_t prigroup);
  AFIO_MAPR |= AFIO_MAPR_SWJ_CFG_JTAG_OFF_SW_ON;

  usart_setup();
  can_setup();

  cm_mask_interrupts(true);
  systick_set_frequency(SYSTEM_TICKS_PER_SEC, 24000000);
  systick_interrupt_enable();
  systick_counter_enable();

  nvic_set_priority(NVIC_PENDSV_IRQ, 0xFF);
  nvic_set_priority(NVIC_SYSTICK_IRQ, 0xFE);

  gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ,
      GPIO_CNF_OUTPUT_PUSHPULL, GPIO8 );

  gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_50_MHZ,
      GPIO_CNF_OUTPUT_PUSHPULL,
      GPIO12 | GPIO13 | GPIO14 );

  //Buttons
  gpio_set_mode(GPIOB, GPIO_MODE_INPUT, GPIO_CNF_INPUT_PULL_UPDOWN, 
      GPIO3|GPIO4);
  gpio_set(GPIOB, GPIO3|GPIO4);


  tim2_setup();
  ws2812b_setup();

  //LED
  gpio_set_mode(GPIOC, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, GPIO13);
  gpio_clear(GPIOC, GPIO13);

  /* extra gpo/NSS B3...B6
     gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_PUSHPULL,
     GPIO3|GPIO3|GPIO5|GPIO6);
     gpio_set(GPIOB, GPIO3|GPIO4|GPIO5|GPIO6);

     inputs (GPI) A0...A3 
     gpio_set_mode(GPIOA, GPIO_MODE_INPUT, GPIO_CNF_INPUT_PULL_UPDOWN, 
     GPIO0|GPIO1|GPIO2|GPIO3);
     gpio_clear(GPIOC, GPIO0|GPIO1|GPIO2|GPIO3);
     */

#if DEBUG
  usart_send_blocking(USART1, '\r');
  usart_send_blocking(USART1, '\n');
  usart_send_blocking(USART1, 'p');
  usart_send_blocking(USART1, 'r');
  usart_send_blocking(USART1, 'e');
  usart_send_blocking(USART1, 'v');
  usart_send_blocking(USART1, 'e');
  usart_send_blocking(USART1, 'd');
  usart_send_blocking(USART1, '\r');
  usart_send_blocking(USART1, '\n');
#endif

  /*
     nvic_enable_irq(NVIC_USB_LP_CAN_RX0_IRQ);
     nvic_enable_irq(NVIC_USB_WAKEUP_IRQ);
     */

  TIM3_CCR1 = 200;
  TIM3_CCR2 = 200;
  TIM3_CCR3 = 200;


  TIM4_CCR1 = 200;
  TIM4_CCR2 = 200;
  TIM4_CCR3 = 200;

  gpio_set(GPIOA, GPIO15);
  if(atomOSInit(idle_stack, sizeof(idle_stack), FALSE) != ATOM_OK) 
    fault(1);
  if (atomQueueCreate (&uart1_rx, uart1_rx_storage, sizeof(uint8_t), 
        sizeof(uart1_rx_storage)) != ATOM_OK) 
    fault(2);
  if (atomQueueCreate (&uart1_tx, uart1_tx_storage, sizeof(uint8_t), 
        sizeof(uart1_tx_storage)) != ATOM_OK) 
    fault(3);
  if (atomQueueCreate (&buttons_q, buttons_storage, sizeof(uint8_t), 
        sizeof(buttons_storage)) != ATOM_OK) 
    fault(4);

  atomThreadCreate(&scan_thread_tcb, 10, scan_thread, 0,
      scan_thread_stack, sizeof(scan_thread_stack), TRUE);

  atomThreadCreate(&master_thread_tcb, 10, master_thread, 0,
      master_thread_stack, sizeof(master_thread_stack), TRUE);

  atomOSStart();
  while (1){
  }
}

int s_write(int file, char *ptr, int len) {
    return u_write(file, (uint8_t *)ptr,len);
};

#if DEBUG
int u_write(int file, uint8_t *ptr, int len) {
    int i;
    for (i = 0; i < len; i++){
        switch(file){
            case 1: //DEBUG
                atomQueuePut(&uart1_tx,-1, (uint8_t*) &ptr[i]);
                break;
        }
    }
    switch(file){
        case 1:
            USART_CR1(USART1) |= USART_CR1_TXEIE;
            break;
    }
    return i;
}
#else
int u_write(int file, uint8_t *ptr, int len) {
  (void)file;
  (void)ptr;
  (void)len;
  return 0;
}
#endif

