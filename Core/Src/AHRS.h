/*
 * AHRS.h
 *
 *  Created on: Feb 28, 2026
 *      Author: Bacnk
 */

#ifndef SRC_AHRS_H_
#define SRC_AHRS_H_

void MadgwickQuaternionUpdate(float ax, float ay, float az, float gx, float gy,
                              float gz, float mx, float my, float mz,
                              float deltat);
void MahonyQuaternionUpdate(float ax, float ay, float az, float gx, float gy,
                            float gz, float mx, float my, float mz,
                            float deltat);



#endif /* SRC_AHRS_H_ */
