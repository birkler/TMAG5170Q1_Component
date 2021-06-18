#ifndef TMAG5170Q1_INTERFACE
#define TMAG5170Q1_INTERFACE
#include <cstring>


#define CRCPP_USE_NAMESPACE
#define CRCPP_USE_CPP11
#define CRCPP_INCLUDE_ESOTERIC_CRC_DEFINITIONS
#include "CRC.h"

extern "C" void TMAG_TransferFrame(const uint8_t tx[4], uint8_t rx[4]);



namespace TMAG5170Q1 {
    

//#define ENUM enum class
#define ENUM enum 

class TMAG5170Q1Device {
public:
    typedef uint8_t CRC;

    ENUM RESERVED {
        
    };
    ENUM RW  {
        READ = 1,
        WRITE = 0
    };
    ENUM START_CONVERSION {
        NO_CONVERSION = 0,
        START_AT_CS_LOW = 1
    };

    ENUM STAT012_INFO {
        SET_COUNT = 0,
        DATA_TYPE = 1
    };

    ENUM ADDRESS {
        DEVICE_CONFIG = 0x0,  //  Configure Device Operation Modes Go
        SENSOR_CONFIG = 0x1, //   Configure Device Operation Modes Go
        SYSTEM_CONFIG = 0x2, //   Configure Device Operation Modes Go
        ALERT_CONFIG = 0x3, //  Configure Device Operation Modes Go
        X_THRX_CONFIG = 0x4, //   Configure Device Operation Modes Go
        Y_THRX_CONFIG = 0x5, //   Configure Device Operation Modes Go
        Z_THRX_CONFIG = 0x6, //   Configure Device Operation Modes Go
        T_THRX_CONFIG = 0x7, //   Configure Device Operation Modes Go
        CONV_STATUS = 0x8,  //   Conversion Satus Register Go
        X_CH_RESULT = 0x9,  //   Conversion Result Register Go
        Y_CH_RESULT = 0xA,  //   Conversion Result Register Go
        Z_CH_RESULT = 0xB,  //   Conversion Result Register Go
        TEMP_RESULT = 0xC,  //   Conversion Result Register Go
        AFE_STATUS = 0xD,  //   Safety Check Satus Register Go
        SYS_STATUS = 0xE,  //   Safety Check Satus Register Go
        TEST_CONFIG = 0xF,  //   Test Configuration Register Go
        OSC_MONITOR = 0x10,  //   Conversion Result Register Go
        MAG_GAIN_CONFIG = 0x11,  //   Configure Device Operation Modes Go
        ANGLE_RESULT = 0x13,  //   Conversion Result Register Go
        MAGNITUDE_RESULT = 0x14, //  Conversion Result Register Go
        LAST_ADDRESS
    };


    union  __attribute__((packed))  Data {
        struct {

        } device_config_;

        struct {
            int16_t value_ : 16;
        } result_;
        struct __attribute__((packed))  {
            unsigned int degrees_:12;
            unsigned int fraction_:4;
        } angle_;

        struct {
            int8_t high_;
            int8_t low_;
        } threshold_; //X_THRX_CONFIG,Y_THRX_CONFIG,Z_THRX_CONFIG,T_THRX_CONFIG

    };

    struct __attribute__((packed))  TXFrame {
        ADDRESS address_ : 7;
        RW rw_ : 1; //0:write 1:read
        Data data_;
        CRC  crc_:4;
        START_CONVERSION cmd0_start_conversion_ : 1; //1=> start conversion when CS goes low
        STAT012_INFO cmd1_data_type_in_stat : 1; //0: STAT[0..2] = SET_COUNT[0..2]  1: STAT[0..2] = DATA_TYPE[0..2]
        RESERVED cmd2_reserved_ : 1;
        RESERVED cmd3_reserved_ : 1;
    };

    static_assert(sizeof(TXFrame) == sizeof(uint32_t),"Boo!");

    struct __attribute__((packed))  RXFrame {
        bool alert_temp_ : 1;
        unsigned int alert_z_ : 1;
        unsigned int alert_y_ : 1;
        unsigned int alert_x_ : 1;
        unsigned int alert_1_ : 1;
        unsigned int alert_0_ : 1;
        unsigned int cfg_reset_ : 1;
        unsigned int prev_crc_status_ : 1;
        Data data_;
        CRC crc_:4;
        unsigned int stat012_ : 3; //see TXFrame::cmd1_data_type_in_stat
        unsigned int error_status_ : 1;
    };
    static_assert(sizeof(RXFrame) == sizeof(uint32_t),"Boo!");


public:
    TXFrame txbuf_;
    RXFrame rxbuf_;
    Data datamem[LAST_ADDRESS];

    CRC calculate_crc(uint8_t msg[4]) {

        // Second argument is the number of bits. The input data must
        // be a whole number of bytes. Pad any used bits with zeros.

        uint8_t message[4] = {msg[0],msg[1],msg[2],msg[3]};
        message[0] ^= 0xF0;
        std::uint32_t crc = CRCPP::CRC::CalculateBits(message, 24, CRCPP::CRC::CRC_4_ITU(),(unsigned char)0x00);
        return crc;
    
    }



    void test_frame() {
        memset(&txbuf_,0,sizeof(txbuf_));
        memset(&rxbuf_,0,sizeof(rxbuf_));
        uint8_t* p_tx = reinterpret_cast< uint8_t* >(&txbuf_);

        txbuf_.rw_ = RW::WRITE;
        //transfer_frame();
        txbuf_.rw_ = RW::READ;

        txbuf_.crc_ = calculate_crc(p_tx);
        //p_tx[0] = 0x12;p_tx[1] = 0x34;p_tx[2] = 0x56;p_tx[3] = 0x78;
        p_tx[0] = 0xe0; p_tx[1] = 0x00; p_tx[2] = 0x00; p_tx[3] = 0x8a; //Send frame with valid crc"

        transfer_frame(false);

        p_tx[0] = 0x60; p_tx[1] = 0x00; p_tx[2] = 0x00; p_tx[3] = 0x8c; //Send frame with valid crc"
    
        transfer_frame(false);

        Data data;
        data.threshold_.low_ = 23;
        data.threshold_.high_ = 23;
        write_data(ADDRESS::X_THRX_CONFIG,data);

        read_data(ADDRESS::X_THRX_CONFIG);

    }

    void write_data(ADDRESS address, Data data) {
        memset(&txbuf_,0,sizeof(txbuf_));
        txbuf_.data_ = data;
        txbuf_.rw_ = RW::WRITE;
        txbuf_.address_ = address;
        transfer_frame();
        datamem[address] = rxbuf_.data_;
    }


    void read_data(ADDRESS address) {
        memset(&txbuf_,0,sizeof(txbuf_));
        txbuf_.rw_ = RW::READ;
        txbuf_.address_ = address;
        transfer_frame();
        datamem[address] = rxbuf_.data_;
    }

    unsigned int to_bits(CRC crc) {
        unsigned int res = 0x0;
        if (crc & 0x1) res += 0x1;
        if (crc & 0x2) res += 0x10;
        if (crc & 0x4) res += 0x100;
        if (crc & 0x8) res += 0x1000;
        return res;
    }
    void transfer_frame(bool updatecrc = true) {
        uint8_t* p_tx = reinterpret_cast< uint8_t* >(&txbuf_);
        if (updatecrc) {
            txbuf_.crc_ = calculate_crc(p_tx);
        }

        uint8_t crc_calc = calculate_crc(p_tx);

        uint8_t* p_rx = reinterpret_cast<uint8_t* >(&rxbuf_);
        TMAG_TransferFrame(p_tx,p_rx);
        printf("tx:%02x%02x%02x%02x val=%8d crc=%04x crc_calc=%04x -> ",
            p_tx[0],p_tx[1],p_tx[2],p_tx[3],
            (int)txbuf_.data_.result_.value_,
            to_bits(txbuf_.crc_),
            to_bits(crc_calc));

        printf("rx:%02x%02x%02x%02x crc=%d reset=%d val=%8d err_stat=%d crc=%04x\n",
            p_rx[0],p_rx[1],p_rx[2],p_rx[3],
            rxbuf_.prev_crc_status_, 
            rxbuf_.cfg_reset_,
            rxbuf_.data_.result_.value_,
            rxbuf_.error_status_,
            to_bits(rxbuf_.crc_));
    }



};


}




#endif //#ifndef TMAG5170Q1_INTERFACE
