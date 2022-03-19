#include <stdio.h>
#include <stdlib.h>
 
int main(int argc, char *argv[]) {
     unsigned char hz[3];
     unsigned char hzk[16][2];
     int i,j;
     FILE *fp;
     if ((fp = fopen("HZK16", "r")) == NULL) {
         printf("Can't open ASC12!\n");
     } else {
         printf("请输入一个汉字:");
         scanf("%s", hz);
         printf("0x%02x,0x%02x\n", hz[0], hz[1]);
         /*
          * 减0xa0是因为EUC表示法中对数据进行了加上0xa0的处理,
          * 减1是因为数组是以0为开始而区号位号是以1为开始的,
          * 乘32是因为16*16点阵每个汉字占用16*16bit=16*2byte空间.
          */
         fseek(fp, (94 * ((hz[0] - 0xa0) - 1) + ((hz[1] - 0xa0) - 1)) * 32, SEEK_SET);
         fread(hzk,32,1,fp);
         for (i = 0; i < 16; i++) {
             for (j = 0; j < 2; j++) {
                 printf("0x%02x\t", hzk[i][j]);
             }
             for (j = 0; j < 2; j++) {
                 printf("%c%c%c%c%c%c%c%c", ((hzk[i][j] >> 7) & 0x01) + ' ', 
                        ((hzk[i][j] >> 6) & 0x01) + ' ',
                        ((hzk[i][j] >> 5) & 0x01) + ' ',
                        ((hzk[i][j] >> 4) & 0x01) + ' ',
                        ((hzk[i][j] >> 3) & 0x01) + ' ',
                        ((hzk[i][j] >> 2) & 0x01) + ' ',
                        ((hzk[i][j] >> 1) & 0x01) + ' ',
                        ((hzk[i][j] >> 0) & 0x01) + ' ');
             }
             putchar('\n');
         }
         fclose(fp);
     }
     system("PAUSE");
     return 0;
}