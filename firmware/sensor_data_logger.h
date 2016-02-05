#ifndef sensor_data_logger_h
#define sensor_data_logger_h

#include <stdio.h>

typedef struct sensor_data_logger_s {
} sensor_data_logger_t;

void open()

void close()
void write(sensor_type, data, numOfData);
void read(sensor_type, data, numOfData);
#endif /* sensor_data_logger_h */
