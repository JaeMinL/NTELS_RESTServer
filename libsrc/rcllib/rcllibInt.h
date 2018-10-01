#ifndef _RCLLIB_INT_H_
#define _RCLLIB_INT_H_

#ifdef __cplusplus
extern "C" {
#endif

#define RCL_LOG(LEVEL,...){\
    fprintf(stderr,__VA_ARGS__);\
}

#ifdef __cplusplus
}
#endif

#endif /* _RCLLIB_INT_H_ */
