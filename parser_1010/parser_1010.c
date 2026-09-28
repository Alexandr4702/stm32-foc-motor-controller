/*
 * parser_1010.c
 *
 *  Created on: Sep 11, 2019
 *      Author: gilg
 */

#include "parser_1010.h"

/*checksum
 *
 */
uint8_t checksum(void *data, uint16_t size)
{
    uint8_t crc = 0;
    uint16_t i;
    for (i = 0; i < size; i++)
    {
        crc ^= ((uint8_t *)data)[i];
    }
    return crc;
}

/* Parser for messages formatted as 0x10 0x10 data crc.
 * data - data to parse
 * size - available data size; updated with the number of bytes left
 * return - message ID and status flags
 */
uint8_t parser(uint8_t *const data, uint16_t *size, messageStack *stack)
{

    static uint8_t ident_cnt = 0; /*if high byte is equal 1 we are reading message */
    static uint16_t need_read = 0;
    static uint8_t id_reading_message = 0x00;

    for (uint16_t i = 0; i < *size; i++)
    {
        if (data[i] == 0x10 && ident_cnt == 0)
        {
            ident_cnt++;
            continue;
        }
        if (data[i] == 0x10 && ident_cnt == 1)
        {
            ident_cnt = 0x80;
            continue;
        }
        if (ident_cnt == 0x80)
        {

            if (need_read > 0)
            {

                int16_t we_can_read_now = *size - need_read;
                switch (id_reading_message)
                {
                case defaultMessageId:
                {
                    if (we_can_read_now >= 0) // read full message
                    {
                        memcpy(((uint8_t *)&stack->defaultMessage_) + sizeof(defaultMessage) -
                                   need_read,
                               data, need_read);
                        ident_cnt = 0;
                        *size = *size - i - need_read;
                        need_read = 0;
                        uint8_t crc = checksum(&stack->defaultMessage_, sizeof(defaultMessage) - 1);
                        return (id_reading_message & 0x3f) | (we_can_read_now > 0) << 7 |
                               (crc == stack->defaultMessage_.crc) << 6;
                    }
                    else
                    {
                        memcpy(((uint8_t *)&stack->defaultMessage_) + sizeof(defaultMessage) -
                                   need_read,
                               data, *size - i);
                        need_read = -we_can_read_now;
                        *size = 0;
                        return 0;
                    }
                    break;
                }

                case McdataId:
                {
                    if (we_can_read_now >= 0) // read full message
                    {
                        memcpy(((uint8_t *)&stack->Mcdata_) + sizeof(Mcdata) - need_read, data,
                               need_read);
                        ident_cnt = 0;
                        *size = *size - i - need_read;
                        need_read = 0;
                        uint8_t crc = checksum(&stack->Mcdata_, sizeof(Mcdata) - 1);
                        return (id_reading_message & 0x3f) | (we_can_read_now > 0) << 7 |
                               (crc == stack->Mcdata_.crc) << 6;
                    }
                    else
                    {
                        memcpy(((uint8_t *)&stack->Mcdata_) + sizeof(Mcdata) - need_read, data,
                               *size - i);
                        need_read = -we_can_read_now;
                        *size = 0;
                        return 0;
                    }
                    break;
                }
                default:
                    ident_cnt = 0;
                    break;
                }
            }

            switch (data[i])
            {
            // in case default mess
            case defaultMessageId:
            {
                need_read = sizeof(defaultMessage);
                int16_t we_can_read_now = *size - i - need_read;
                if (we_can_read_now >= 0) // read full message
                {
                    memcpy(&stack->defaultMessage_, data + i, sizeof(defaultMessage));
                    ident_cnt = 0;
                    *size = *size - i - need_read;
                    need_read = 0;
                    uint8_t crc = checksum(&stack->defaultMessage_, sizeof(defaultMessage) - 1);
                    return (data[i] & 0x3f) | (we_can_read_now > 0) << 7 |
                           (crc == stack->defaultMessage_.crc) << 6;
                }
                else
                {
                    memcpy(&stack->defaultMessage_, data + i,
                           *size - i); // read avaliable part message
                    id_reading_message = data[i];
                    *size = 0;
                    need_read = -we_can_read_now;
                    return 0;
                }
                break;
            }
            // in case McData
            // Id--------------------------------------------------------------------------
            case McdataId:
            {
                need_read = sizeof(Mcdata);
                int16_t we_can_read_now = *size - i - need_read;
                if (we_can_read_now >= 0) // read full message
                {
                    memcpy(&stack->Mcdata_, data + i, sizeof(Mcdata));
                    ident_cnt = 0;
                    *size = *size - i - need_read;
                    need_read = 0;
                    uint8_t crc = checksum(&stack->Mcdata_, sizeof(Mcdata) - 1);
                    return (data[i] & 0x3f) | (we_can_read_now > 0) << 7 |
                           (crc == stack->Mcdata_.crc) << 6;
                }
                else
                {
                    memcpy(&stack->Mcdata_, data + i, *size - i); // read avaliable part message
                    id_reading_message = data[i];
                    *size = 0;
                    need_read = -we_can_read_now;
                    return 0;
                }
                break;
            }
            //---------------------------------------------------------------------------------
            default:
                ident_cnt = 0;
                break;
            }
        }
    }
    return 0;
}

/* Generate a message for transmission, adding the 0x10 0x10 prefix and CRC.
 * message - output buffer of at least size + 2 bytes
 * data - data to transmit
 * size - data size
 */

void generate_message(void *message, void *const data, uint8_t id, uint16_t size)
{
    ((char *)message)[0] = 0x10;
    ((char *)message)[1] = 0x10;
    ((char *)message)[2] = id;
    memcpy((uint8_t *)message + 3, (const uint8_t *)data + 1, size - 1);
    ((char *)message)[size + 1] = checksum((uint8_t *)message + 2, size - 1);
}
