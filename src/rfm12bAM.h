/*
 * rfm12bAm.h
 *
 *  Created on: 01.09.2018
 *      Author: tomek
 */

#ifndef RFM12BAM_H_
#define RFM12BAM_H_

#ifdef __cplusplus
extern "C" {
#endif

void rfInitAM() ;

void rfSend(unsigned char data);
void rfSendString(char *s) ;



#ifdef __cplusplus
}
#endif

#endif /* RFM12BAM_H_ */
