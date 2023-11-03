#include <neo_ch32v003tim.h>

volatile NEO_STATE _neo_state = NEO_IDLE;
unsigned char *_neo_buf;
unsigned short _neo_len = 0;
volatile unsigned char _neo_bitcount = 1;
volatile unsigned short _neo_bytecount = 0;
unsigned short _neo_period;
NEO_CONTINUES _neo_continues = NEO_SINGLE;
volatile unsigned char _neo_sending = 0x00;

unsigned short _neo_onetime = 300;
unsigned short _neo_zerotime = 60;
unsigned short _neo_latchtime = 0x0fff;

void neo_beginBuff(unsigned char *p, unsigned short len, NEO_CONTINUES cont)
{
    _neo_buf = p;
    _neo_len = len;
    _neo_continues = cont;

    neo_begin();

    if (_neo_continues == NEO_CONSTANT)
    {
        TIM_ITConfig(TIM1, TIM_IT_Update, ENABLE);
        NVIC_EnableIRQ(TIM1_UP_IRQn);
        NVIC_SetPriority(TIM1_UP_IRQn, 0x00);
        _neo_sending = 0xff;
    }
}

void neo_begin()
{
    TIM_TimeBaseInitTypeDef neotim;
    TIM_OCInitTypeDef neooc;
    GPIO_InitTypeDef neogpio;

    // so these values seem to work with all the ws2812's/sk's i have laying around.
    // its a (little) out of spec(slow), +-300khz instead of 400khz.
    _neo_period = 160;
    _neo_onetime = 70;
    _neo_zerotime = 140;
    _neo_latchtime = 8000;

    neotim.TIM_Period = _neo_period;
    neotim.TIM_Prescaler = 0x00;
    neotim.TIM_ClockDivision = TIM_CKD_DIV1;
    neotim.TIM_RepetitionCounter = 0;
    neotim.TIM_CounterMode = TIM_CounterMode_Up;

    neooc.TIM_OCMode = TIM_OCMode_PWM1;
    neooc.TIM_OutputState = TIM_OutputState_Enable;
    neooc.TIM_OutputNState = TIM_OutputNState_Enable;
    neooc.TIM_Pulse = 0;
    neooc.TIM_OCPolarity = TIM_OCPolarity_Low;
    neooc.TIM_OCNPolarity = TIM_OCNPolarity_High;
    neooc.TIM_OCIdleState = TIM_OCIdleState_Reset;
    neooc.TIM_OCNIdleState = TIM_OCNIdleState_Reset;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_TIM1, ENABLE);

    neogpio.GPIO_Pin = GPIO_Pin_4;
    neogpio.GPIO_Speed = GPIO_Speed_50MHz;
    neogpio.GPIO_Mode = GPIO_Mode_AF_PP;

    GPIO_Init(GPIOC, &neogpio);
    TIM_OC4Init(TIM1, &neooc);

    TIM_TimeBaseInit(TIM1, &neotim);

    // NVIC_SetPriority(TIM1_UP_IRQn, 0x00); // 00 is highest?
    TIM_CCxCmd(TIM1, TIM_Channel_4, TIM_CCx_Enable);

    TIM_CtrlPWMOutputs(TIM1, ENABLE);
    TIM_Cmd(TIM1, ENABLE);
}

unsigned char neo_start()
{
    if (!_neo_len)
        return 1;
    if (_neo_state == NEO_IDLE)
    {
        _neo_sending = 0xff;

        TIM1->CH4CVR = 0xffff;
        TIM1->ATRLR = _neo_period;

        TIM_ITConfig(TIM1, TIM_IT_Update, ENABLE);

        NVIC_EnableIRQ(TIM1_UP_IRQn);
        // still set?
        // NVIC_SetPriority(TIM1_UP_IRQn, 0x00);

        return 0;
    }
    else
        return 1;
}

unsigned char neo_startBuff(unsigned char *p, unsigned short len, NEO_CONTINUES cont)
{
    unsigned char r;
    // turn of continues if on
    _neo_continues = NEO_SINGLE;
    while (_neo_state != NEO_IDLE)
        ; // block untile neoframe is complete
    _neo_continues = cont;
    _neo_len = len;
    _neo_buf = p;
    r = neo_start();
    return r;
}

unsigned char neo_sending()
{
    return _neo_sending;
}

// it think using the noneos spl/hal is slower thank read and wirting the register directly?
extern void TIM1_UP_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void TIM1_UP_IRQHandler(void)
{
    TIM1->CH4CVR = 0xffff;
    // TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
    TIM1->INTFR = ~(TIM_IT_Update);

    switch (_neo_state)
    {
    case NEO_IDLE:
        _neo_bytecount = 0;
        _neo_bitcount = 1;
        TIM1->ATRLR = _neo_period;
        _neo_state = NEO_DATA;
        break;

        break;

    case NEO_DATA:

        switch (_neo_bitcount)
        {
        case 0:
            _neo_bitcount = 1;
        case 1:
            if (_neo_buf[_neo_bytecount] & (0x80))
                TIM1->CH4CVR = _neo_onetime;
            else
                TIM1->CH4CVR = _neo_zerotime;
            break;

        case 2:
            if (_neo_buf[_neo_bytecount] & (0x40))
                TIM1->CH4CVR = _neo_onetime;
            else
                TIM1->CH4CVR = _neo_zerotime;
            break;

        case 3:
            if (_neo_buf[_neo_bytecount] & (0x20))
                TIM1->CH4CVR = _neo_onetime;
            else
                TIM1->CH4CVR = _neo_zerotime;
            break;

        case 4:
            if (_neo_buf[_neo_bytecount] & (0x10))
                TIM1->CH4CVR = _neo_onetime;
            else
                TIM1->CH4CVR = _neo_zerotime;
            break;

        case 5:
            if (_neo_buf[_neo_bytecount] & (0x08))
                TIM1->CH4CVR = _neo_onetime;
            else
                TIM1->CH4CVR = _neo_zerotime;
            break;

        case 6:
            if (_neo_buf[_neo_bytecount] & (0x04))
                TIM1->CH4CVR = _neo_onetime;
            else
                TIM1->CH4CVR = _neo_zerotime;
            break;

        case 7:
            if (_neo_buf[_neo_bytecount] & (0x02))
                TIM1->CH4CVR = _neo_onetime;
            else
                TIM1->CH4CVR = _neo_zerotime;
            break;

        case 8:
            if (_neo_buf[_neo_bytecount] & (0x01))
                TIM1->CH4CVR = _neo_onetime;
            else
                TIM1->CH4CVR = _neo_zerotime;
            _neo_bitcount = 0;
            _neo_bytecount++;
            if (_neo_bytecount >= _neo_len)
            {
                _neo_state = NEO_LATCH;
            }
            break;
        default:
            break;
        }
        _neo_bitcount++;

        // // switchcase above if faster than these ifs
        // if (0xff & (neo_buf[_neo_bytecount] & (0x80 >> _neo_bitcount)))
        //     // TIM_SetCompare2(NEO_ONETIME);
        //     TIM1->CH1CVR = _neo_onetime;
        // else
        //     // TIM_SetCompare2(NEO_ZEROTIME);
        //     TIM1->CH1CVR = _neo_zerotime;
        // _neo_bitcount++;
        // // _neo_bitcount &= 0x07;
        // if (_neo_bitcount > 7)
        // { // just overflown
        //     _neo_bitcount = 0;
        //     _neo_bytecount++;
        //     if (_neo_bytecount >= neo_len)
        //     {
        //         neo_state = NEO_LATCH;
        //     }
        // }
        break;

    case NEO_LATCH:
        TIM1->CH4CVR = 0xffff;

        // TIM1->CH1CVR = ;
        TIM1->ATRLR = _neo_latchtime;
        _neo_state = NEO_IDLE;
        if (_neo_continues == NEO_SINGLE)
        {
            TIM_ITConfig(TIM1, TIM_IT_Update, DISABLE);
            _neo_sending = 0x00;
        }
        break;

    default:
        break;
    }
}