#ifndef _COMMON_H_
#define _COMMON_H_

#ifndef BLOCKSDS
// Use iprintf/iscanf on NDS
#define sscanf siscanf
#define printf iprintf
#define fprintf fiprintf
#define sprintf siprintf
#define snprintf sniprintf
#define vsnprintf vsniprintf
#endif

#endif /* _COMMON_H_ */
