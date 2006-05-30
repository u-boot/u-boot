#ifndef IxOsalOsAssert_H
#define IxOsalOsAssert_H

#define IX_OSAL_OS_ASSERT(c)    if(!(c)) \
                                { \
                                    ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDOUT, "Assertion failure \n", 0, 0, 0, 0, 0, 0);\
                                    while(1); \
                                }

#endif /* IxOsalOsAssert_H */
