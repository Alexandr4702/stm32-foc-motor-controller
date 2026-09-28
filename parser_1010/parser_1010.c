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

/*parser for message format 0x10 0x10 data crc
 *data - data for parsing
 *size - size's data for parsing after calling function return how mutch left
 *return - return id message if after pars message left bytes high byte is 1
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

/*
 *generate message  for sending (adds 0x10 0x10 to beggin and calculate crc in penultimate bytes  )
 *message - pointer at message have to be bigger then size+2
 *data - data for transimtting
 *size - size data for transimtting
 */

void generate_message(void *message, void *const data, uint8_t id, uint16_t size)
{
    ((char *)message)[0] = 0x10;
    ((char *)message)[1] = 0x10;
    ((char *)message)[2] = id;
    memcpy((uint8_t *)message + 3, (const uint8_t *)data + 1, size - 1);
    ((char *)message)[size + 1] = checksum((uint8_t *)message + 2, size - 1);
}
