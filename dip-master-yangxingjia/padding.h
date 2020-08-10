#ifndef PADDING_H
#define PADDING_H
#include <qglobal.h>
enum BorderType {
  BORDER_REPLICATE,     // aaaaaa|abcdefgh|hhhhhhh
  BORDER_REFLECT,       // fedcba|abcdefgh|hgfedcb
  BORDER_REFLECT_101,   // gfedcb|abcdefgh|gfedcba
  BORDER_WRAP,          // cdefgh|abcdefgh|abcdefg
  BORDER_CONSTANT,      // iiiiii|abcdefgh|iiiiiii  with some specified 'i'
};

void copyMakeBorder(uchar *src, int w, int h, int cn, int top, int bottom,
                         int left, int right, BorderType borderType, const uchar *value, uchar *dst);
void copyRemoveBorder(const uchar *src, int w, int h, int cn, int top, int bottom,
                    int left, int right,  uchar *dst);
#endif // PADDING_H
