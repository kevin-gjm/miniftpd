#include "common.h"
#include "str.h"



void str_trim_crlf(char *str)
{
        char *p = &str[strlen(str)-1];
        while(*p == '\r' || *p == '\n')
        {
                *p ='\0';
                p--;
        }

}
void str_split(const char *str,char *left,char *right,char c)
{
        char *p = strchr(str, c);
        if(p==NULL)
        {
                strcpy(left, str);
                right = NULL;
        }else{
                strncpy(left, str, p-str);
                strcpy(right,p+1);
        }
}
int str_all_space(const char *str)
{
        while (*str)
        {
                if(!isspace(*str))
                        return 0;
                str++;
        }
        return 1;
}
void str_upper(char *str)
{
        while(*str)
        {
                *str = toupper(*str);
                str++;
        }
}
/*
  使用的算法:
  12345678
  8*1     +
  7*10    +
  6*10*10 +
  ....
*/
long long str_to_longlong(const char *str)
{
        //atoll 并非所有的系统都提供
        //return atoll(str);
/*
  unsigned int i;
  for(i=0;i<len;i++)
  {
  char ch = str[len-i-1];
  long long val;
  if(ch < '0' || ch > '9')
  return 0;
  val = ch - '0';
  val *= mult;
  result += val;
  mult *= 10;
  }
*/
        long long result=0;
        long long mult = 1;
        unsigned int len = strlen(str);
        int i; //不能是无符号整型数字,后面与0进行比较
        if(len > 15)
                return 0;
        for(i=len-1;i>=0;i--)
        {
                char ch = str[i];
                long long val = 0;
                if(ch < '0' || ch > '9')
                        return 0;
                val = ch - '0';
                val *= mult;
                result += val;
                mult *= 10;
        }
        return result;
}

/*
  将八进制字符串转换为无符号整型
  可以使用与上面的方法相同的逻辑

  123456712
  0 + 1
  1*8 + 2
  (1*8 + 2)*8 + 3
  ......
*/
unsigned int str_octal_to_uint(const char *str)
{
        unsigned int result = 0;
        int seen_none_zero_digit = 0;
        while(*str)
        {
                int digit = *str;
                if(!isdigit(digit) || digit > '7')
                        break;
                if(digit != '0')
                        seen_none_zero_digit = 1;
                if(seen_none_zero_digit)
                {
                        result <<= 3;//*8
                        result += (digit - '0');
                }
                str++;
        }

        return result;

}
