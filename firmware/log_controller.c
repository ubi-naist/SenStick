#include "log_controller.h"


void logControllerGetLogCount(int magicWord, const log_address_info_t *p_address_info)
{}

void openLog(log_context_t *p_context, uint8_t logID, int magicWord, const log_address_info_t *p_address_info)
{}

void closeLog(log_context_t *p_context)
{}

int writeLog(log_context_t *p_context, uint8_t *p_data, int length)
{
    return 0;
}

int readLog(log_context_t *p_context, uint8_t *p_data, int length)
{
    return 0;
}

int seekLogReadPosition(log_context_t *p_context, int position)
{
    return 0;
}
