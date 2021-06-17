#ifndef TMAG5170Q1_INTERFACE
#define TMAG5170Q1_INTERFACE
#include <cstring>

extern "C" void TMAG_TransferFrame(const uint8_t tx[4], uint8_t rx[4]);

namespace TMAG5170Q1 {
    
class TMAG5170Q1Device {
public:
    enum RW {
        READ = 0,
        WRITE = 1
    };

    struct __attribute__((packed))  TXFrame {
        unsigned int address_ : 7;
        RW rw_ : 1;
        int16_t value_ : 16;
        unsigned int  crc_:4;
        unsigned int cmd0_ : 1;
        unsigned int cmd1_ : 1;
        unsigned int cmd2_ : 1;
        unsigned int cmd3_ : 1;
    };

    static_assert(sizeof(TXFrame) == sizeof(uint32_t),"Boo!");

    struct __attribute__((packed))  RXFrame {
        unsigned int alert_temp_ : 1;
        unsigned int alert_z_ : 1;
        unsigned int alert_y_ : 1;
        unsigned int alert_x_ : 1;
        unsigned int alert_1_ : 1;
        unsigned int alert_0_ : 1;
        unsigned int cfg_reset_ : 1;
        unsigned int prev_crc_status_ : 1;
        union {
            int16_t value_ : 16;
            struct __attribute__((packed))  {
                unsigned int degrees_:12;
                unsigned int fraction_:4;
            } angle_;
        };
        unsigned int crc_:4;
        unsigned int status012_ : 3;
        unsigned int error_status_ : 1;
    };
    static_assert(sizeof(RXFrame) == sizeof(uint32_t),"Boo!");


public:
    TXFrame txbuf_;
    RXFrame rxbuf_;

    uint8_t calculate_crc(uint8_t message[4]) {
        enum {
            POLYNOMIAL = 0b10011<<27  /* x^4 + x + 1 */
        };


        /*
        * Initially, the dividend is the remainder.
        */
        uint32_t remainder = 
            message[0] << 24 | 
            message[1] << 16 | 
            message[2] << 8 | 
            message[3] << 0;

        /*
        * For each bit position in the message....
        */
        for (uint8_t bit = 24; bit > 0; --bit)
        {
            /*
            * If the uppermost bit is a 1...
            */
            if (remainder & 0x80000000)
            {
                /*
                * XOR the previous remainder with the divisor.
                */
                remainder ^= POLYNOMIAL;
            }

            /*
            * Shift the next bit of the message into the remainder.
            */
            remainder <<= 1;
        }

        /*
        * Return only the relevant bits of the remainder as CRC.
        */
        uint32_t result = remainder >> 27;
        result &= 0xF;
        result ^= 0xF;

        return result;
    }



    void test_frame() {
        memset(&txbuf_,0,sizeof(txbuf_));
        memset(&rxbuf_,0,sizeof(rxbuf_));
        txbuf_.rw_ = RW::WRITE;
        transfer_frame();
        txbuf_.rw_ = RW::READ;
        transfer_frame();
    }


    void transfer_frame() {
        uint8_t* p_tx = reinterpret_cast< uint8_t* >(&txbuf_);
        txbuf_.crc_ = calculate_crc(p_tx);
        //p_tx[0] = 0x12;p_tx[1] = 0x34;p_tx[2] = 0x56;p_tx[3] = 0x78;
        p_tx[0] = 0xe0; p_tx[1] = 0x00; p_tx[2] = 0x00; p_tx[3] = 0x8a; //Send frame with valid crc"

        p_tx[0] = 0x60; p_tx[1] = 0x00; p_tx[2] = 0x00; p_tx[3] = 0x8c; //Send frame with valid crc"



        uint8_t crc_calc = calculate_crc(p_tx);

        uint8_t* p_rx = reinterpret_cast<uint8_t* >(&rxbuf_);
        TMAG_TransferFrame(p_tx,p_rx);
        printf("tx:%02x%02x%02x%02x val=%8d crc=%04x crc_calc=%04x -> ",
            p_tx[0],p_tx[1],p_tx[2],p_tx[3],
            (int)txbuf_.value_,
            (unsigned int)txbuf_.crc_,
            (unsigned int) crc_calc);

        printf("rx:%02x%02x%02x%02x crc=%d reset=%d val=%8d err_stat=%d crc=%04x\n",
            p_rx[0],p_rx[1],p_rx[2],p_rx[3],
            rxbuf_.prev_crc_status_, 
            rxbuf_.cfg_reset_,
            rxbuf_.value_,
            rxbuf_.error_status_,
            rxbuf_.crc_);
    }



};


}




#endif //#ifndef TMAG5170Q1_INTERFACE
