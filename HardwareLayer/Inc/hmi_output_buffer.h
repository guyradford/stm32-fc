//
// Created by guyra on 19/02/2023.
//

#ifndef STM32_FC_HMI_OUTPUT_BUFFER_H
#define STM32_FC_HMI_OUTPUT_BUFFER_H

void HMIOutput_AddToBuffer(char *string, int len);
void HMIOutput_SendNext(void );
void HMIOutput_OnSendComplete(void);

#endif //STM32_FC_HMI_OUTPUT_BUFFER_H
