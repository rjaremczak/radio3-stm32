//
// Created by Robert Jaremczak on 2017.10.05.
//

#include "UsbDevice.h"
#include "usb_pwr.h"

__IO uint16_t wIstr;  /* ISTR register last read value */
__IO uint8_t bIntPackSOF = 0;  /* SOFs received between 2 consecutive packets */
__IO uint32_t esof_counter = 0; /* expected SOF counter */
__IO uint32_t wCNTR = 0;
__IO uint16_t SaveRState;
__IO uint16_t SaveTState;

namespace {
    UsbDevice *_usbDevicePtr;
}

extern "C" void USB_LP_CAN1_RX0_IRQHandler(void) {
    _usbDevicePtr->istr();
}

extern "C" void USBWakeUp_IRQHandler(void) {
    EXTI_ClearITPendingBit(EXTI_Line18);
}

UsbDevice::UsbDevice() {
    _usbDevicePtr = this;

    epIn[0] = &UsbDevice::ep1in;
    epIn[1] = &UsbDevice::ep2in;
    epIn[2] = &UsbDevice::ep3in;
    epIn[3] = &UsbDevice::ep4in;
    epIn[4] = &UsbDevice::ep5in;
    epIn[5] = &UsbDevice::ep6in;
    epIn[6] = &UsbDevice::ep7in;

    epOut[0] = &UsbDevice::ep1out;
    epOut[1] = &UsbDevice::ep2out;
    epOut[2] = &UsbDevice::ep3out;
    epOut[3] = &UsbDevice::ep4out;
    epOut[4] = &UsbDevice::ep5out;
    epOut[5] = &UsbDevice::ep6out;
    epOut[6] = &UsbDevice::ep7out;
}

void UsbDevice::istr() {
    wIstr = _GetISTR();

    if (wIstr & ISTR_SOF & wInterrupt_Mask) handleStartOfFrame();
    if (wIstr & ISTR_CTR & wInterrupt_Mask) handleCorrectTransfer();
    if (wIstr & ISTR_RESET & wInterrupt_Mask) handleReset();
    if (wIstr & ISTR_ERR & wInterrupt_Mask) handleError();
    if (wIstr & ISTR_WKUP & wInterrupt_Mask) handleWakeUp();
    if (wIstr & ISTR_SUSP & wInterrupt_Mask) handleSuspend();
    if (wIstr & ISTR_ESOF & wInterrupt_Mask) handleExpectedStartOfFrame();
}

void UsbDevice::ep1in() {}
void UsbDevice::ep2in() {}
void UsbDevice::ep3in() {}
void UsbDevice::ep4in() {}
void UsbDevice::ep5in() {}
void UsbDevice::ep6in() {}
void UsbDevice::ep7in() {}

void UsbDevice::ep1out() {}
void UsbDevice::ep2out() {}
void UsbDevice::ep3out() {}
void UsbDevice::ep4out() {}
void UsbDevice::ep5out() {}
void UsbDevice::ep6out() {}
void UsbDevice::ep7out() {}

void UsbDevice::initRCC() {
    RCC_USBCLKConfig(RCC_USBCLKSource_PLLCLK_1Div5);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USB, ENABLE);
}

void UsbDevice::initNVIC() {
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    NVIC_InitStructure.NVIC_IRQChannel = USB_LP_CAN1_RX0_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    /* Enable the USB Wake-up interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = USBWakeUp_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_Init(&NVIC_InitStructure);
}

void UsbDevice::handleStartOfFrame() {
    _SetISTR((uint16_t) CLR_SOF);
    bIntPackSOF++;
}

void UsbDevice::handleReset() {
    _SetISTR((uint16_t) CLR_RESET);
    Device_Property.Reset();
}

void UsbDevice::handleError() {
    _SetISTR((uint16_t) CLR_ERR);
}

void UsbDevice::handleWakeUp() {
    _SetISTR((uint16_t) CLR_WKUP);
    Resume(RESUME_EXTERNAL);
}

void UsbDevice::handleSuspend() {

    /* check if SUSPEND is possible */
    if (fSuspendEnabled) {
        Suspend();
    } else {
        /* if not possible then resume after xx ms */
        Resume(RESUME_LATER);
    }
    /* clear of the ISTR bit must be done after setting of CNTR_FSUSP */
    _SetISTR((uint16_t) CLR_SUSP);
}

void UsbDevice::handleExpectedStartOfFrame() {
    __IO uint32_t EP[8];

    /* clear ESOF flag in ISTR */
    _SetISTR((uint16_t) CLR_ESOF);

    if ((_GetFNR() & FNR_RXDP) != 0) {
        /* increment ESOF counter */
        esof_counter++;

        /* test if we enter in ESOF more than 3 times with FSUSP =0 and RXDP =1=>> possible missing SUSP flag*/
        if ((esof_counter > 3) && ((_GetCNTR() & CNTR_FSUSP) == 0)) {
            /* this a sequence to apply a force RESET*/

            /*Store CNTR value */
            wCNTR = _GetCNTR();

            /*Store endpoints registers status */
            for (auto i = 0; i < 8; i++) EP[i] = _GetENDPOINT(i);

            /*apply FRES */
            wCNTR |= CNTR_FRES;
            _SetCNTR(wCNTR);

            /*clear FRES*/
            wCNTR &= ~CNTR_FRES;
            _SetCNTR(wCNTR);

            /*poll for RESET flag in ISTR*/
            while ((_GetISTR() & ISTR_RESET) == 0);

            /* clear RESET flag in ISTR */
            _SetISTR((uint16_t) CLR_RESET);

            /*restore Enpoints*/
            for (auto i = 0; i < 8; i++)
                _SetENDPOINT(i, EP[i]);

            esof_counter = 0;
        }
    } else {
        esof_counter = 0;
    }

    /* resume handling timing is made with ESOFs */
    Resume(RESUME_ESOF); /* request without change of the machine state */
}

void UsbDevice::handleCorrectTransfer() {
    __IO uint16_t wEPVal = 0;
    /* stay in loop while pending interrupts */
    while (((wIstr = _GetISTR()) & ISTR_CTR) != 0) {
        /* extract highest priority endpoint number */
        EPindex = (uint8_t) (wIstr & ISTR_EP_ID);
        if (EPindex == 0) {
            /* Decode and service control endpoint interrupt */
            /* calling related service routine */
            /* (Setup0_Process, In0_Process, Out0_Process) */

            /* save RX & TX status */
            /* and set both to NAK */

            SaveRState = _GetENDPOINT(ENDP0);
            SaveTState = SaveRState & EPTX_STAT;
            SaveRState &= EPRX_STAT;

            _SetEPRxTxStatus(ENDP0, EP_RX_NAK, EP_TX_NAK);

            /* DIR bit = origin of the interrupt */

            if ((wIstr & ISTR_DIR) == 0) {
                /* DIR = 0 */

                /* DIR = 0      => IN  int */
                /* DIR = 0 implies that (EP_CTR_TX = 1) always  */

                _ClearEP_CTR_TX(ENDP0);
                In0_Process();

                /* before terminate set Tx & Rx status */

                _SetEPRxTxStatus(ENDP0, SaveRState, SaveTState);
                return;
            } else {
                /* DIR = 1 */

                /* DIR = 1 & CTR_RX       => SETUP or OUT int */
                /* DIR = 1 & (CTR_TX | CTR_RX) => 2 int pending */

                wEPVal = _GetENDPOINT(ENDP0);

                if ((wEPVal & EP_SETUP) != 0) {
                    _ClearEP_CTR_RX(ENDP0); /* SETUP bit kept frozen while CTR_RX = 1 */
                    Setup0_Process();
                    /* before terminate set Tx & Rx status */

                    _SetEPRxTxStatus(ENDP0, SaveRState, SaveTState);
                    return;
                } else if ((wEPVal & EP_CTR_RX) != 0) {
                    _ClearEP_CTR_RX(ENDP0);
                    Out0_Process();
                    /* before terminate set Tx & Rx status */

                    _SetEPRxTxStatus(ENDP0, SaveRState, SaveTState);
                    return;
                }
            }
        }/* if(EPindex == 0) */
        else {
            /* Decode and service non control endpoints interrupt  */

            /* process related endpoint register */
            wEPVal = _GetENDPOINT(EPindex);
            if ((wEPVal & EP_CTR_RX) != 0) {
                /* clear int flag */
                _ClearEP_CTR_RX(EPindex);

                /* call OUT service function */
                //(*pEpInt_OUT[EPindex - 1])();
                (this->*epOut[EPindex - 1])();

            } /* if((wEPVal & EP_CTR_RX) */

            if ((wEPVal & EP_CTR_TX) != 0) {
                /* clear int flag */
                _ClearEP_CTR_TX(EPindex);

                /* call IN service function */
                //(*pEpInt_IN[EPindex - 1])();
                (this->*epIn[EPindex - 1])();

            } /* if((wEPVal & EP_CTR_TX) != 0) */

        }/* if(EPindex == 0) else */

    }/* while(...) */
}